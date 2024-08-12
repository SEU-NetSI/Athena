#include <string.h>
#include <math.h>

#include "swarm_ranging.h"
#include "adhocuwb_impl.h"

#ifndef RANGING_DEBUG_ENABLE
#undef DEBUG_PRINT
#define DEBUG_PRINT
#endif

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

static uint16_t MY_UWB_ADDRESS = 1;

static QueueHandle_t rxQueue;
static Neighbor_Set_t neighborSet;
static TimerHandle_t neighborSetEvictionTimer;
NO_DMA_CCM_SAFE_ZERO_INIT Ranging_Table_Set_t rangingTableSet;
static TimerHandle_t rangingTableSetEvictionTimer;
static UWB_Message_Listener_t listener;
static TaskHandle_t uwbRangingTxTaskHandle = 0;
static TaskHandle_t uwbRangingRxTaskHandle = 0;
static int TfBufferIndex = 0;
static Timestamp_Tuple_t TfBuffer[Tf_BUFFER_POOL_SIZE] = {0};
static SemaphoreHandle_t TfBufferMutex;
static int rangingSeqNumber = 1;
static float velocity;
static Ranging_Table_t EMPTY_RANGING_TABLE = {
    .neighborAddress = UWB_DEST_EMPTY,
    .Rp.timestamp.full = 0,
    .Rp.seqNumber = 0,
    .Tp.timestamp.full = 0,
    .Tp.seqNumber = 0,
    .Rf.timestamp.full = 0,
    .Rf.seqNumber = 0,
    .Tf.timestamp.full = 0,
    .Tf.seqNumber = 0,
    .Re.timestamp.full = 0,
    .Re.seqNumber = 0,
    .latestReceived.timestamp.full = 0,
    .latestReceived.seqNumber = 0,
    .TrRrBuffer.cur = 0,
    .TrRrBuffer.latest = 0,
    .state = RANGING_STATE_S1,
    .period = RANGING_PERIOD,
    .nextExpectedDeliveryTime = M2T(RANGING_PERIOD),
    .expirationTime = M2T(RANGING_TABLE_HOLD_TIME),
    .lastSendTime = 0,
    .distance = -1
};

int16_t distanceTowards[NEIGHBOR_ADDRESS_MAX + 1] = {[0 ... NEIGHBOR_ADDRESS_MAX] = -1};

int16_t getDistance(UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  return distanceTowards[neighborAddress];
}

void setDistance(UWB_Address_t neighborAddress, int16_t distance) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  distanceTowards[neighborAddress] = distance;
}

void rangingTableBufferInit(Ranging_Table_Tr_Rr_Buffer_t *rangingTableBuffer) {
  rangingTableBuffer->cur = 0;
  rangingTableBuffer->latest = 0;
  Timestamp_Tuple_t empty = {.seqNumber = 0, .timestamp.full = 0};
  for (set_index_t i = 0; i < Tr_Rr_BUFFER_POOL_SIZE; i++) {
    rangingTableBuffer->candidates[i].Tr = empty;
    rangingTableBuffer->candidates[i].Rr = empty;
  }
}

void rangingTableBufferUpdate(Ranging_Table_Tr_Rr_Buffer_t *rangingTableBuffer,
                              Timestamp_Tuple_t Tr,
                              Timestamp_Tuple_t Rr) {
  rangingTableBuffer->candidates[rangingTableBuffer->cur].Tr = Tr;
  rangingTableBuffer->candidates[rangingTableBuffer->cur].Rr = Rr;
  // shift
  rangingTableBuffer->latest = rangingTableBuffer->cur;
  rangingTableBuffer->cur = (rangingTableBuffer->cur + 1) % Tr_Rr_BUFFER_POOL_SIZE;
}

Ranging_Table_Tr_Rr_Candidate_t rangingTableBufferGetCandidate(Ranging_Table_Tr_Rr_Buffer_t *rangingTableBuffer,
                                                               Timestamp_Tuple_t Tf) {
  set_index_t index = rangingTableBuffer->latest;
  uint64_t rightBound = Tf.timestamp.full % UWB_MAX_TIMESTAMP;
  Ranging_Table_Tr_Rr_Candidate_t candidate = {.Rr.timestamp.full = 0, .Tr.timestamp.full = 0};

  for (int count = 0; count < Tr_Rr_BUFFER_POOL_SIZE; count++) {
    if (rangingTableBuffer->candidates[index].Rr.timestamp.full &&
        rangingTableBuffer->candidates[index].Rr.timestamp.full % UWB_MAX_TIMESTAMP < rightBound) {
      candidate.Tr = rangingTableBuffer->candidates[index].Tr;
      candidate.Rr = rangingTableBuffer->candidates[index].Rr;
      break;
    }
    index = (index - 1 + Tr_Rr_BUFFER_POOL_SIZE) % Tr_Rr_BUFFER_POOL_SIZE;
  }

  return candidate;
}

void updateTfBuffer(Timestamp_Tuple_t timestamp) {
  xSemaphoreTake(TfBufferMutex, portMAX_DELAY);
  TfBufferIndex++;
  TfBufferIndex %= Tf_BUFFER_POOL_SIZE;
  TfBuffer[TfBufferIndex] = timestamp;
//  DEBUG_PRINT("updateTfBuffer: time = %llu, seq = %d\n", TfBuffer[TfBufferIndex].timestamp.full, TfBuffer[TfBufferIndex].seqNumber);
  xSemaphoreGive(TfBufferMutex);
}

Timestamp_Tuple_t findTfBySeqNumber(uint16_t seqNumber) {
  xSemaphoreTake(TfBufferMutex, portMAX_DELAY);
  Timestamp_Tuple_t Tf = {.timestamp.full = 0, .seqNumber = 0};
  int startIndex = TfBufferIndex;
  /* Backward search */
  for (int i = startIndex; i >= 0; i--) {
    if (TfBuffer[i].seqNumber == seqNumber) {
      Tf = TfBuffer[i];
      break;
    }
  }
  if (!Tf.timestamp.full) {
    /* Forward search */
    for (int i = startIndex + 1; i < Tf_BUFFER_POOL_SIZE; i++) {
      if (TfBuffer[i].seqNumber == seqNumber) {
        Tf = TfBuffer[i];
        break;
      }
    }
  }
  xSemaphoreGive(TfBufferMutex);
  return Tf;
}

Timestamp_Tuple_t getLatestTxTimestamp() {
  return TfBuffer[TfBufferIndex];
}

void getLatestNTxTimestamps(Timestamp_Tuple_t *timestamps, int n) {
  ASSERT(n <= Tf_BUFFER_POOL_SIZE);
  xSemaphoreTake(TfBufferMutex, portMAX_DELAY);
  int startIndex = (TfBufferIndex + 1 - n + Tf_BUFFER_POOL_SIZE) % Tf_BUFFER_POOL_SIZE;
  for (int i = n - 1; i >= 0; i--) {
    timestamps[i] = TfBuffer[startIndex];
    startIndex = (startIndex + 1) % Tf_BUFFER_POOL_SIZE;
  }
  xSemaphoreGive(TfBufferMutex);
}

Ranging_Table_Set_t *getGlobalRangingTableSet() {
  return &rangingTableSet;
}

void rangingTableInit(Ranging_Table_t *table, UWB_Address_t neighborAddress) {
  memset(table, 0, sizeof(Ranging_Table_t));
  table->state = RANGING_STATE_S1;
  table->neighborAddress = neighborAddress;
  table->period = RANGING_PERIOD;
  table->nextExpectedDeliveryTime = 0;
  table->expirationTime = 0;
  table->lastSendTime = 0;
  rangingTableBufferInit(&table->TrRrBuffer); // Can be safely removed this line since memset() is called
}

/* Ranging Table Set Operations */
void rangingTableSetInit(Ranging_Table_Set_t *set) {
  set->mu = xSemaphoreCreateMutex();
  set->size = 0;
  for (int i = 0; i < RANGING_TABLE_SIZE_MAX; i++) {
    set->tables[i] = EMPTY_RANGING_TABLE;
  }
}

static void rangingTableSetSwapTable(Ranging_Table_Set_t *set, int first, int second) {
  Ranging_Table_t temp = set->tables[first];
  set->tables[first] = set->tables[second];
  set->tables[second] = temp;
}

static int rangingTableSetSearchTable(Ranging_Table_Set_t *set, UWB_Address_t targetAddress) {
  /* Binary Search */
  int left = -1, right = set->size, res = -1;
  while (left + 1 != right) {
    int mid = left + (right - left) / 2;
    if (set->tables[mid].neighborAddress == targetAddress) {
      res = mid;
      break;
    } else if (set->tables[mid].neighborAddress > targetAddress) {
      right = mid;
    } else {
      left = mid;
    }
  }
  return res;
}

typedef int (*rangingTableCompareFunc)(Ranging_Table_t *, Ranging_Table_t *);

static int COMPARE_BY_ADDRESS(Ranging_Table_t *first, Ranging_Table_t *second) {
  if (first->neighborAddress == second->neighborAddress) {
    return 0;
  }
  if (first->neighborAddress > second->neighborAddress) {
    return 1;
  }
  return -1;
}

static int COMPARE_BY_EXPIRATION_TIME(Ranging_Table_t *first, Ranging_Table_t *second) {
  if (first->expirationTime == second->expirationTime) {
    return 0;
  }
  if (first->expirationTime > second->expirationTime) {
    return -1;
  }
  return 1;
}

static int COMPARE_BY_NEXT_EXPECTED_DELIVERY_TIME(Ranging_Table_t *first, Ranging_Table_t *second) {
  if (first->nextExpectedDeliveryTime == second->nextExpectedDeliveryTime) {
    return 0;
  }
  if (first->nextExpectedDeliveryTime > second->nextExpectedDeliveryTime) {
    return 1;
  }
  return -1;
}

static int COMPARE_BY_LAST_SEND_TIME(Ranging_Table_t *first, Ranging_Table_t *second) {
  if (first->lastSendTime == second->lastSendTime) {
    return 0;
  }
  if (first->lastSendTime > second->lastSendTime) {
    return 1;
  }
  return -1;
}

/* Build the heap */
static void rangingTableSetArrange(Ranging_Table_Set_t *set, int index, int len, rangingTableCompareFunc compare) {
  int leftChild = 2 * index + 1;
  int rightChild = 2 * index + 2;
  int maxIndex = index;
  if (leftChild < len && compare(&set->tables[maxIndex], &set->tables[leftChild]) < 0) {
    maxIndex = leftChild;
  }
  if (rightChild < len && compare(&set->tables[maxIndex], &set->tables[rightChild]) < 0) {
    maxIndex = rightChild;
  }
  if (maxIndex != index) {
    rangingTableSetSwapTable(set, index, maxIndex);
    rangingTableSetArrange(set, maxIndex, len, compare);
  }
}

/* Sort the ranging table */
static void rangingTableSetRearrange(Ranging_Table_Set_t *set, rangingTableCompareFunc compare) {
  /* Build max heap */
  for (int i = set->size / 2 - 1; i >= 0; i--) {
    rangingTableSetArrange(set, i, set->size, compare);
  }
  for (int i = set->size - 1; i >= 0; i--) {
    rangingTableSetSwapTable(set, 0, i);
    rangingTableSetArrange(set, 0, i, compare);
  }
}

static int rangingTableSetClearExpire(Ranging_Table_Set_t *set) {
  Time_t curTime = xTaskGetTickCount();
  int evictionCount = 0;

  for (int i = 0; i < rangingTableSet.size; i++) {
    if (rangingTableSet.tables[i].expirationTime <= curTime) {
      DEBUG_PRINT("rangingTableSetClearExpire: Clean ranging table for neighbor %u that expire at %lu.\n",
                  rangingTableSet.tables[i].neighborAddress,
                  rangingTableSet.tables[i].expirationTime);
      setDistance(rangingTableSet.tables[i].neighborAddress, -1);
      rangingTableSet.tables[i] = EMPTY_RANGING_TABLE;
      evictionCount++;
    }
  }
  /* Keeps ranging table set in order. */
  rangingTableSetRearrange(&rangingTableSet, COMPARE_BY_ADDRESS);
  rangingTableSet.size -= evictionCount;

  return evictionCount;
}

static void rangingTableSetClearExpireTimerCallback(TimerHandle_t timer) {
  xSemaphoreTake(rangingTableSet.mu, portMAX_DELAY);

  Time_t curTime = xTaskGetTickCount();
  DEBUG_PRINT("rangingTableSetClearExpireTimerCallback: Trigger expiration timer at %lu.\n", curTime);

  int evictionCount = rangingTableSetClearExpire(&rangingTableSet);
  if (evictionCount > 0) {
    DEBUG_PRINT("rangingTableSetClearExpireTimerCallback: Evict total %d ranging tables.\n", evictionCount);
  } else {
    DEBUG_PRINT("rangingTableSetClearExpireTimerCallback: Evict none.\n");
  }

  xSemaphoreGive(rangingTableSet.mu);
}

bool rangingTableSetAddTable(Ranging_Table_Set_t *set, Ranging_Table_t table) {
  int index = rangingTableSetSearchTable(set, table.neighborAddress);
  if (index != -1) {
    DEBUG_PRINT(
        "rangingTableSetAddTable: Try to add an already added ranging table for neighbor %u, update it instead.\n",
        table.neighborAddress);
    set->tables[index] = table;
    return true;
  }
  /* If ranging table is full now and there is no expired ranging table, then ignore. */
  if (set->size == RANGING_TABLE_SIZE_MAX && rangingTableSetClearExpire(&rangingTableSet) == 0) {
    DEBUG_PRINT("rangingTableSetAddTable: Ranging table if full, ignore new neighbor %u.\n",
                table.neighborAddress);
    return false;
  }
  /* Add the new entry to the last */
  uint8_t curIndex = set->size;
  set->tables[curIndex] = table;
  set->size++;
  /* Sort the ranging table, keep it in order. */
  rangingTableSetRearrange(set, COMPARE_BY_ADDRESS);
  DEBUG_PRINT("rangingTableSetAddTable: Add new neighbor %u to ranging table.\n", table.neighborAddress);
  return true;
}

void rangingTableSetUpdateTable(Ranging_Table_Set_t *set, Ranging_Table_t table) {
  int index = rangingTableSetSearchTable(set, table.neighborAddress);
  if (index == -1) {
    DEBUG_PRINT("rangingTableSetUpdateTable: Cannot find correspond table for neighbor %u, add it instead.\n",
                table.neighborAddress);
    rangingTableSetAddTable(set, table);
  } else {
    set->tables[index] = table;
    DEBUG_PRINT("rangingTableSetUpdateTable: Update table for neighbor %u.\n", table.neighborAddress);
  }
}

void rangingTableSetRemoveTable(Ranging_Table_Set_t *set, UWB_Address_t neighborAddress) {
  if (set->size == 0) {
    DEBUG_PRINT("rangingTableSetRemoveTable: Ranging table is empty, ignore.\n");
    return;
  }
  int index = rangingTableSetSearchTable(set, neighborAddress);
  if (index == -1) {
    DEBUG_PRINT("rangingTableSetRemoveTable: Cannot find correspond table for neighbor %u, ignore.\n", neighborAddress);
    return;
  }
  rangingTableSetSwapTable(set, index, set->size - 1);
  set->tables[set->size - 1] = EMPTY_RANGING_TABLE;
  set->size--;
  rangingTableSetRearrange(set, COMPARE_BY_ADDRESS);
}

Ranging_Table_t rangingTableSetFindTable(Ranging_Table_Set_t *set, UWB_Address_t neighborAddress) {
  int index = rangingTableSetSearchTable(set, neighborAddress);
  Ranging_Table_t table = EMPTY_RANGING_TABLE;
  if (index == -1) {
    DEBUG_PRINT("rangingTableSetFindTable: Cannot find correspond table for neighbor %u.\n", neighborAddress);
  } else {
    table = set->tables[index];
  }
  return table;
}

void neighborBitSetInit(Neighbor_Bit_Set_t *bitSet) {
  bitSet->bits = 0;
  bitSet->size = 0;
}

void neighborBitSetAdd(Neighbor_Bit_Set_t *bitSet, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  uint64_t prevBits = bitSet->bits;
  bitSet->bits |= (1ULL << neighborAddress);
  if (prevBits != bitSet->bits) {
    bitSet->size++;
  }
}

void neighborBitSetRemove(Neighbor_Bit_Set_t *bitSet, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  uint64_t prevBits = bitSet->bits;
  bitSet->bits &= ~(1ULL << neighborAddress);
  if (prevBits != bitSet->bits) {
    bitSet->size--;
  }
}

void neighborBitSetClear(Neighbor_Bit_Set_t *bitSet) {
  bitSet->bits = 0;
  bitSet->size = 0;
}

bool neighborBitSetHas(Neighbor_Bit_Set_t *bitSet, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  return (bitSet->bits & (1ULL << neighborAddress)) != 0;
}

Neighbor_Set_t *getGlobalNeighborSet() {
  return &neighborSet;
}

void neighborSetInit(Neighbor_Set_t *set) {
  set->size = 0;
  set->mu = xSemaphoreCreateMutex();
  neighborBitSetInit(&set->oneHop);
  neighborBitSetInit(&set->twoHop);
  set->neighborNewHooks.hook = NULL;
  set->neighborNewHooks.next = NULL;
  set->neighborExpirationHooks.hook = NULL;
  set->neighborExpirationHooks.next = NULL;
  set->neighborTopologyChangeHooks.hook = NULL;
  set->neighborTopologyChangeHooks.next = NULL;
  for (UWB_Address_t neighborAddress = 0; neighborAddress <= NEIGHBOR_ADDRESS_MAX; neighborAddress++) {
    set->expirationTime[neighborAddress] = 0;
    neighborBitSetInit(&set->twoHopReachSets[neighborAddress]);
  }
}

bool neighborSetHas(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  return neighborBitSetHas(&set->oneHop, neighborAddress) || neighborBitSetHas(&set->twoHop, neighborAddress);
}

bool neighborSetHasOneHop(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  return neighborBitSetHas(&set->oneHop, neighborAddress);
}

bool neighborSetHasTwoHop(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  return neighborBitSetHas(&set->twoHop, neighborAddress);
}

void neighborSetAddOneHopNeighbor(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  bool isNewNeighbor = false;
  if (!neighborSetHas(set, neighborAddress)) {
    isNewNeighbor = true;
  }
  /* If neighbor is previous two-hop neighbor, remove it from two-hop neighbor set. */
  if (neighborSetHasTwoHop(set, neighborAddress)) {
    neighborSetRemoveNeighbor(set, neighborAddress);
  }
  /* Add one-hop neighbor. */
  if (!neighborSetHasOneHop(set, neighborAddress)) {
    neighborBitSetAdd(&set->oneHop, neighborAddress);
    neighborSetUpdateExpirationTime(set, neighborAddress);
    neighborSetHooksInvoke(&set->neighborTopologyChangeHooks, neighborAddress);
  }
  set->size = set->oneHop.size + set->twoHop.size;
  if (isNewNeighbor) {
    neighborSetHooksInvoke(&set->neighborNewHooks, neighborAddress);
  }
}

void neighborSetAddTwoHopNeighbor(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  bool isNewNeighbor = false;
  if (!neighborSetHas(set, neighborAddress)) {
    isNewNeighbor = true;
  }
  /* If neighbor is previous one-hop neighbor, remove it from one-hop neighbor set. */
  if (neighborSetHasOneHop(set, neighborAddress)) {
    neighborSetRemoveNeighbor(set, neighborAddress);
  }
  if (!neighborSetHasTwoHop(set, neighborAddress)) {
    /* Add two-hop neighbor. */
    neighborBitSetAdd(&set->twoHop, neighborAddress);
    neighborSetUpdateExpirationTime(set, neighborAddress);
    neighborSetHooksInvoke(&set->neighborTopologyChangeHooks, neighborAddress);
  }
  set->size = set->oneHop.size + set->twoHop.size;
  if (isNewNeighbor) {
    neighborSetHooksInvoke(&set->neighborNewHooks, neighborAddress);
  }
}

void neighborSetRemoveNeighbor(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  if (neighborSetHas(set, neighborAddress)) {
    if (neighborSetHasOneHop(set, neighborAddress) && neighborSetHasTwoHop(set, neighborAddress)) {
      ASSERT(0); // impossible
    }
    set->expirationTime[neighborAddress] = 0;
    if (neighborSetHasOneHop(set, neighborAddress)) {
      neighborBitSetRemove(&set->oneHop, neighborAddress);
      /* Remove related path to two-hop neighbor */
      for (UWB_Address_t twoHopNeighbor = 0; twoHopNeighbor <= NEIGHBOR_ADDRESS_MAX; twoHopNeighbor++) {
        if (neighborSetHasRelation(set, neighborAddress, twoHopNeighbor)) {
          neighborSetRemoveRelation(set, neighborAddress, twoHopNeighbor);
        }
      }
    } else if (neighborSetHasTwoHop(set, neighborAddress)) {
      neighborBitSetRemove(&set->twoHop, neighborAddress);
      /* Clear related two-hop reach set */
      neighborBitSetClear(&set->twoHopReachSets[neighborAddress]);
    } else {
      ASSERT(0); // impossible
    }
    neighborSetHooksInvoke(&set->neighborTopologyChangeHooks, neighborAddress);
  }
  set->size = set->oneHop.size + set->twoHop.size;
}

bool neighborSetHasRelation(Neighbor_Set_t *set, UWB_Address_t from, UWB_Address_t to) {
  ASSERT(from <= NEIGHBOR_ADDRESS_MAX);
  ASSERT(to <= NEIGHBOR_ADDRESS_MAX);
  return neighborBitSetHas(&set->twoHopReachSets[to], from);
}

void neighborSetAddRelation(Neighbor_Set_t *set, UWB_Address_t from, UWB_Address_t to) {
  ASSERT(from <= NEIGHBOR_ADDRESS_MAX);
  ASSERT(to <= NEIGHBOR_ADDRESS_MAX);
  if (!neighborBitSetHas(&set->twoHopReachSets[to], from)) {
    neighborBitSetAdd(&set->twoHopReachSets[to], from);
    neighborSetHooksInvoke(&set->neighborTopologyChangeHooks, from);
  }
}

void neighborSetRemoveRelation(Neighbor_Set_t *set, UWB_Address_t from, UWB_Address_t to) {
  ASSERT(from <= NEIGHBOR_ADDRESS_MAX);
  ASSERT(to <= NEIGHBOR_ADDRESS_MAX);
  if (neighborBitSetHas(&set->twoHopReachSets[to], from)) {
    neighborBitSetRemove(&set->twoHopReachSets[to], from);
    neighborSetHooksInvoke(&set->neighborTopologyChangeHooks, from);
  }
}

void neighborSetRegisterNewNeighborHook(Neighbor_Set_t *set, neighborSetHook hook) {
  ASSERT(hook);
  Neighbor_Set_Hooks_t cur = {
      .hook = hook,
      .next = (struct Neighbor_Set_Hook_Node *) set->neighborNewHooks.hook
  };
  set->neighborNewHooks = cur;
}

void neighborSetRegisterExpirationHook(Neighbor_Set_t *set, neighborSetHook hook) {
  ASSERT(hook);
  Neighbor_Set_Hooks_t cur = {
      .hook = hook,
      .next = (struct Neighbor_Set_Hook_Node *) set->neighborExpirationHooks.hook
  };
  set->neighborExpirationHooks = cur;
}

void neighborSetRegisterTopologyChangeHook(Neighbor_Set_t *set, neighborSetHook hook) {
  ASSERT(hook);
  Neighbor_Set_Hooks_t cur = {
      .hook = hook,
      .next = (struct Neighbor_Set_Hook_Node *) set->neighborTopologyChangeHooks.hook
  };
  set->neighborTopologyChangeHooks = cur;
}

void neighborSetHooksInvoke(Neighbor_Set_Hooks_t *hooks, UWB_Address_t neighborAddress) {
  neighborSetHook cur = hooks->hook;
  while (cur != NULL) {
    DEBUG_PRINT("neighborSetHooksInvoke: Invoke neighbor set hook.\n");
    cur(neighborAddress);
    cur = (neighborSetHook) hooks->next;
  }
}

void neighborSetUpdateExpirationTime(Neighbor_Set_t *set, UWB_Address_t neighborAddress) {
  ASSERT(neighborAddress <= NEIGHBOR_ADDRESS_MAX);
  set->expirationTime[neighborAddress] = xTaskGetTickCount() + M2T(NEIGHBOR_SET_HOLD_TIME);
}

int neighborSetClearExpire(Neighbor_Set_t *set) {
  Time_t curTime = xTaskGetTickCount();
  int evictionCount = 0;
  for (UWB_Address_t neighborAddress = 0; neighborAddress <= NEIGHBOR_ADDRESS_MAX; neighborAddress++) {
    if (neighborSetHas(set, neighborAddress) && set->expirationTime[neighborAddress] <= curTime) {
      evictionCount++;
      neighborSetRemoveNeighbor(set, neighborAddress);
      DEBUG_PRINT("neighborSetClearExpire: neighbor %u expire at %lu.\n", neighborAddress, curTime);
      neighborSetHooksInvoke(&set->neighborExpirationHooks, neighborAddress);
    }
  }
  return evictionCount;
}

static void topologySensing(Ranging_Message_t *rangingMessage) {
//  DEBUG_PRINT("topologySensing: Received ranging message from neighbor %u.\n", rangingMessage->header.srcAddress);
  UWB_Address_t neighborAddress = rangingMessage->header.srcAddress;
  if (!neighborSetHasOneHop(&neighborSet, neighborAddress)) {
    /* Add current neighbor to one-hop neighbor set. */
    neighborSetAddOneHopNeighbor(&neighborSet, neighborAddress);
  }
  neighborSetUpdateExpirationTime(&neighborSet, neighborAddress);

  /* Infer one-hop and tow-hop neighbors from received ranging message. */
  uint8_t bodyUnitCount = (rangingMessage->header.msgLength - sizeof(Ranging_Message_Header_t)) / sizeof(Body_Unit_t);
  for (int i = 0; i < bodyUnitCount; i++) {
    #ifdef ROUTING_OLSR_ENABLE
    if (rangingMessage->bodyUnits[i].address == uwbGetAddress()) {
      /* If been selected as MPR, add neighbor to mpr selector set. */
      if (rangingMessage->bodyUnits[i].flags.MPR) {
        if (!mprSelectorSetHas(getGlobalMPRSelectorSet(), neighborAddress)) {
          mprSelectorSetAdd(getGlobalMPRSelectorSet(), neighborAddress);
        }
        mprSelectorSetUpdateExpirationTime(getGlobalMPRSelectorSet(), neighborAddress);
      } else {
        if (mprSelectorSetHas(getGlobalMPRSelectorSet(), neighborAddress)) {
          mprSelectorSetRemove(getGlobalMPRSelectorSet(), neighborAddress);
        }
      }
    }
    #endif
    UWB_Address_t twoHopNeighbor = rangingMessage->bodyUnits[i].address;
    if (twoHopNeighbor != uwbGetAddress() && !neighborSetHasOneHop(&neighborSet, twoHopNeighbor)) {
      /* If it is not one-hop neighbor then it is now my two-hop neighbor, if new add it to neighbor set. */
      if (!neighborSetHasTwoHop(&neighborSet, twoHopNeighbor)) {
        neighborSetAddTwoHopNeighbor(&neighborSet, twoHopNeighbor);
      }
      if (!neighborSetHasRelation(&neighborSet, neighborAddress, twoHopNeighbor)) {
        neighborSetAddRelation(&neighborSet, neighborAddress, twoHopNeighbor);
      }
      neighborSetUpdateExpirationTime(&neighborSet, twoHopNeighbor);
    }
  }
}

static void neighborSetClearExpireTimerCallback(TimerHandle_t timer) {
  xSemaphoreTake(neighborSet.mu, portMAX_DELAY);

  Time_t curTime = xTaskGetTickCount();
  DEBUG_PRINT("neighborSetClearExpireTimerCallback: Trigger expiration timer at %lu.\n", curTime);

  int evictionCount = neighborSetClearExpire(&neighborSet);
  if (evictionCount > 0) {
    DEBUG_PRINT("neighborSetClearExpireTimerCallback: Evict total %d neighbors.\n", evictionCount);
  } else {
    DEBUG_PRINT("neighborSetClearExpireTimerCallback: Evict none.\n");
  }

  xSemaphoreGive(neighborSet.mu);
}

void printRangingTable(Ranging_Table_t *table) {
  DEBUG_PRINT("Rp = %u, Tr = %u, Rf = %u, \n",
              table->Rp.seqNumber,
              table->TrRrBuffer.candidates[table->TrRrBuffer.latest].Tr.seqNumber,
              table->Rf.seqNumber);
  DEBUG_PRINT("Tp = %u, Rr = %u, Tf = %u, Re = %u, \n",
              table->Tp.seqNumber,
              table->TrRrBuffer.candidates[table->TrRrBuffer.latest].Rr.seqNumber,
              table->Tf.seqNumber,
              table->Re.seqNumber);
  DEBUG_PRINT("\n");
}

void printRangingTableSet(Ranging_Table_Set_t *set) {
  DEBUG_PRINT("neighbor\t distance\t period\t expire\t \n");
  for (int i = 0; i < set->size; i++) {
    if (set->tables[i].neighborAddress == UWB_DEST_EMPTY) {
      continue;
    }
    DEBUG_PRINT("%u\t %d\t %lu\t %lu\t \n",
                set->tables[i].neighborAddress,
                set->tables[i].distance,
                set->tables[i].period,
                set->tables[i].expirationTime);
  }
  DEBUG_PRINT("---\n");
}

void printRangingMessage(Ranging_Message_t *rangingMessage) {
  for (int i = 0; i < RANGING_MAX_Tr_UNIT; i++) {
    DEBUG_PRINT("lastTxTimestamp %d seq=%u, lastTxTimestamp=%2x%8lx\n",
                i,
                rangingMessage->header.lastTxTimestamps[i].seqNumber,
                rangingMessage->header.lastTxTimestamps[i].timestamp.high8,
                rangingMessage->header.lastTxTimestamps[i].timestamp.low32);
  }
  if (rangingMessage->header.msgLength - sizeof(Ranging_Message_Header_t) == 0) {
    return;
  }
  uint16_t body_unit_number = (rangingMessage->header.msgLength - sizeof(Ranging_Message_Header_t)) / sizeof(Body_Unit_t);
  if (body_unit_number >= RANGING_MAX_BODY_UNIT) {
    DEBUG_PRINT("printRangingMessage: malformed body unit number.\n");
    return;
  }
  for (int i = 0; i < body_unit_number; i++) {
    DEBUG_PRINT("unitAddress=%u, Seq=%u\n",
                rangingMessage->bodyUnits[i].address,
                rangingMessage->bodyUnits[i].timestamp.seqNumber);
  }
}

void printNeighborBitSet(Neighbor_Bit_Set_t *bitSet) {
  DEBUG_PRINT("%u has %u neighbors = ", uwbGetAddress(), bitSet->size);
  for (int neighborAddress = 0; neighborAddress <= NEIGHBOR_ADDRESS_MAX; neighborAddress++) {
    if (neighborBitSetHas(bitSet, neighborAddress)) {
      DEBUG_PRINT("%u ", neighborAddress);
    }
  }
  DEBUG_PRINT("\n");
}

void printNeighborSet(Neighbor_Set_t *set) {
  DEBUG_PRINT("%u has %u one hop neighbors, %u two hop neighbors, %u neighbors in total.\n",
              uwbGetAddress(),
              set->oneHop.size,
              set->twoHop.size,
              set->size
  );
  DEBUG_PRINT("one-hop neighbors = ");
  for (UWB_Address_t oneHopNeighbor = 0; oneHopNeighbor <= NEIGHBOR_ADDRESS_MAX; oneHopNeighbor++) {
    if (neighborBitSetHas(&set->oneHop, oneHopNeighbor)) {
      DEBUG_PRINT("%u ", oneHopNeighbor);
    }
  }
  DEBUG_PRINT("\n");
  DEBUG_PRINT("two-hop neighbors = ");
  for (UWB_Address_t twoHopNeighbor = 0; twoHopNeighbor <= NEIGHBOR_ADDRESS_MAX; twoHopNeighbor++) {
    if (neighborBitSetHas(&set->twoHop, twoHopNeighbor)) {
      DEBUG_PRINT("%u ", twoHopNeighbor);
    }
  }
  DEBUG_PRINT("\n");
  for (UWB_Address_t twoHopNeighbor = 0; twoHopNeighbor <= NEIGHBOR_ADDRESS_MAX; twoHopNeighbor++) {
    if (!neighborBitSetHas(&set->twoHop, twoHopNeighbor)) {
      continue;
    }
    DEBUG_PRINT("to two-hop neighbor %u: ", twoHopNeighbor);
    for (UWB_Address_t oneHopNeighbor = 0; oneHopNeighbor <= NEIGHBOR_ADDRESS_MAX; oneHopNeighbor++) {
      if (neighborSetHasRelation(set, oneHopNeighbor, twoHopNeighbor)) {
        DEBUG_PRINT("%u ", oneHopNeighbor);
      }
    }
    DEBUG_PRINT("\n");
  }
}

static int16_t computeDistance(Timestamp_Tuple_t Tp, Timestamp_Tuple_t Rp,
                               Timestamp_Tuple_t Tr, Timestamp_Tuple_t Rr,
                               Timestamp_Tuple_t Tf, Timestamp_Tuple_t Rf) {

  bool isErrorOccurred = false;

  if (Tp.seqNumber != Rp.seqNumber || Tr.seqNumber != Rr.seqNumber || Tf.seqNumber != Rf.seqNumber) {
    DEBUG_PRINT("Ranging Error: sequence number mismatch\n");
    isErrorOccurred = true;
  }

  if (Tp.seqNumber >= Tf.seqNumber || Rp.seqNumber >= Rf.seqNumber) {
    DEBUG_PRINT("Ranging Error: sequence number out of order\n");
    isErrorOccurred = true;
  }

  int64_t tRound1, tReply1, tRound2, tReply2, diff1, diff2, t;
  tRound1 = (Rr.timestamp.full - Tp.timestamp.full + UWB_MAX_TIMESTAMP) % UWB_MAX_TIMESTAMP;
  tReply1 = (Tr.timestamp.full - Rp.timestamp.full + UWB_MAX_TIMESTAMP) % UWB_MAX_TIMESTAMP;
  tRound2 = (Rf.timestamp.full - Tr.timestamp.full + UWB_MAX_TIMESTAMP) % UWB_MAX_TIMESTAMP;
  tReply2 = (Tf.timestamp.full - Rr.timestamp.full + UWB_MAX_TIMESTAMP) % UWB_MAX_TIMESTAMP;
  diff1 = tRound1 - tReply1;
  diff2 = tRound2 - tReply2;
  t = (diff1 * tReply2 + diff2 * tReply1 + diff2 * diff1) / (tRound1 + tRound2 + tReply1 + tReply2);
  int16_t distance = (int16_t) t * 0.4691763978616;

  if (distance < 0) {
    DEBUG_PRINT("Ranging Error: distance < 0\n");
    isErrorOccurred = true;
  }

  if (distance > 1000) {
    DEBUG_PRINT("Ranging Error: distance > 1000\n");
    isErrorOccurred = true;
  }

  if (isErrorOccurred) {
    return -1;
  }

  return distance;
}

static void S1_Tf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Don't update Tf here since sending message is an async action, we put all Tf in TfBuffer. */
  rangingTable->state = RANGING_STATE_S2;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S1_Tf: S%d -> S%d\n", prevState, curState);
}

static void S1_RX_NO_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  rangingTable->state = RANGING_STATE_S1;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("Invalid state transition occurs, just ignore\n");
//  DEBUG_PRINT("S1_RX_NO_Rf: S%d -> S%d\n", prevState, curState);
}

static void S1_RX_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  rangingTable->state = RANGING_STATE_S1;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("Invalid state transition occurs, just ignore\n");
//  DEBUG_PRINT("S1_RX_Rf: S%d -> S%d\n", prevState, curState);
}

static void S2_Tf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Don't update Tf here since sending message is an async action, we put all Tf in TfBuffer. */
  rangingTable->state = RANGING_STATE_S2;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S2_Tf: S%d -> S%d\n", prevState, curState);
}

static void S2_RX_NO_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  rangingTable->state = RANGING_STATE_S2;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("Invalid state transition occurs, just ignore\n");
//  DEBUG_PRINT("S2_RX_NO_Rf: S%d -> S%d\n", prevState, curState);
}

static void S2_RX_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Find corresponding Tf in TfBuffer, it is possible that can not find corresponding Tf. */
  rangingTable->Tf = findTfBySeqNumber(rangingTable->Rf.seqNumber);
  if (!rangingTable->Tf.timestamp.full) {
    DEBUG_PRINT("Cannot found corresponding Tf in Tf buffer, the ranging frequency may be too high or Tf buffer is in a small size.");
  }

  /* Shift ranging table
   * Rp <- Rf
   * Tp <- Tf  Rr <- Re
   */
  rangingTable->Rp = rangingTable->Rf;
  rangingTable->Tp = rangingTable->Tf;
  rangingTable->TrRrBuffer.candidates[rangingTable->TrRrBuffer.cur].Rr = rangingTable->Re;

  Timestamp_Tuple_t empty = {.timestamp.full = 0, .seqNumber = 0};
  rangingTable->Rf = empty;
  rangingTable->Tf = empty;
  rangingTable->Re = empty;

  rangingTable->state = RANGING_STATE_S3;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S2_RX_Rf: S%d -> S%d\n", prevState, curState);
}

static void S3_Tf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Don't update Tf here since sending message is an async action, we put all Tf in TfBuffer. */
  rangingTable->state = RANGING_STATE_S4;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S3_Tf: S%d -> S%d\n", prevState, curState);
}

static void S3_RX_NO_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Shift ranging table
   * Rr <- Re
   */
  rangingTable->TrRrBuffer.candidates[rangingTable->TrRrBuffer.cur].Rr = rangingTable->Re;
  Timestamp_Tuple_t empty = {.timestamp.full = 0, .seqNumber = 0};
  rangingTable->Re = empty;

  rangingTable->state = RANGING_STATE_S3;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S3_RX_NO_Rf: S%d -> S%d\n", prevState, curState);
}

static void S3_RX_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Shift ranging table
   * Rr <- Re
   */
  rangingTable->TrRrBuffer.candidates[rangingTable->TrRrBuffer.cur].Rr = rangingTable->Re;
  Timestamp_Tuple_t empty = {.timestamp.full = 0, .seqNumber = 0};
  rangingTable->Re = empty;

  rangingTable->state = RANGING_STATE_S3;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S3_RX_Rf: S%d -> S%d\n", prevState, curState);
}

static void S4_Tf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Don't update Tf here since sending message is an async action, we put all Tf in TfBuffer. */
  rangingTable->state = RANGING_STATE_S4;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S4_Tf: S%d -> S%d\n", prevState, curState);
}

static void S4_RX_NO_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Shift ranging table
   * Rr <- Re
   */
  rangingTable->TrRrBuffer.candidates[rangingTable->TrRrBuffer.cur].Rr = rangingTable->Re;
  Timestamp_Tuple_t empty = {.timestamp.full = 0, .seqNumber = 0};
  rangingTable->Re = empty;

  rangingTable->state = RANGING_STATE_S4;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S4_RX_NO_Rf: S%d -> S%d\n", prevState, curState);
}

static void S4_RX_Rf(Ranging_Table_t *rangingTable) {
  RANGING_TABLE_STATE prevState = rangingTable->state;

  /* Find corresponding Tf in TfBuffer, it is possible that can not find corresponding Tf. */
  rangingTable->Tf = findTfBySeqNumber(rangingTable->Rf.seqNumber);

  Ranging_Table_Tr_Rr_Candidate_t Tr_Rr_Candidate = rangingTableBufferGetCandidate(&rangingTable->TrRrBuffer,
                                                                                   rangingTable->Tf);

//  printRangingTable(rangingTable);

  /* try to compute distance */
  int16_t distance = computeDistance(rangingTable->Tp, rangingTable->Rp,
                                     Tr_Rr_Candidate.Tr, Tr_Rr_Candidate.Rr,
                                     rangingTable->Tf, rangingTable->Rf);
  if (distance > 0) {
    rangingTable->distance = distance;
    setDistance(rangingTable->neighborAddress, distance);
  } else {
//    DEBUG_PRINT("distance is not updated since some error occurs\n");
  }

  /* Shift ranging table
   * Rp <- Rf
   * Tp <- Tf  Rr <- Re
   */
  rangingTable->Rp = rangingTable->Rf;
  rangingTable->Tp = rangingTable->Tf;
  rangingTable->TrRrBuffer.candidates[rangingTable->TrRrBuffer.cur].Rr = rangingTable->Re;

  Timestamp_Tuple_t empty = {.timestamp.full = 0, .seqNumber = 0};
  rangingTable->Rf = empty;
  rangingTable->Tf = empty;
  rangingTable->Re = empty;

  // TODO: check if valid
  rangingTable->state = RANGING_STATE_S3;

  RANGING_TABLE_STATE curState = rangingTable->state;
//  DEBUG_PRINT("S4_RX_Rf: S%d -> S%d\n", prevState, curState);
}

/* Don't call this handler function. */
static void S5_Tf(Ranging_Table_t *rangingTable) {
//  DEBUG_PRINT("S5_Tf: invalid handler invocation of temporary state RANGING_STATE_S5\n");
}

/* Don't call this handler function. */
static void S5_RX_NO_Rf(Ranging_Table_t *rangingTable) {
//  DEBUG_PRINT("S5_RX_NO_Rf: invalid handler invocation of temporary state RANGING_STATE_S5\n");
}

/* Don't call this handler function. */
static void S5_RX_Rf(Ranging_Table_t *rangingTable) {
//  DEBUG_PRINT("S5_RX_Rf: invalid handler invocation of temporary state RANGING_STATE_S5\n");
}

/* Don't call this handler function. */
static void RESERVED_STUB(Ranging_Table_t *rangingTable) {
//  DEBUG_PRINT("RESERVED_STUB: Error, been invoked unexpectedly\n");
}

static RangingTableEventHandler EVENT_HANDLER[RANGING_TABLE_STATE_COUNT][RANGING_TABLE_EVENT_COUNT] = {
    {RESERVED_STUB, RESERVED_STUB, RESERVED_STUB},
    {S1_Tf, S1_RX_NO_Rf, S1_RX_Rf},
    {S2_Tf, S2_RX_NO_Rf, S2_RX_Rf},
    {S3_Tf, S3_RX_NO_Rf, S3_RX_Rf},
    {S4_Tf, S4_RX_NO_Rf, S4_RX_Rf},
    /* RANGING_STATE_S5 is effectively a temporary state for distance calculation, never been invoked */
    {S5_Tf, S5_RX_NO_Rf, S5_RX_Rf}
};

void rangingTableOnEvent(Ranging_Table_t *table, RANGING_TABLE_EVENT event) {
  ASSERT(table->state < RANGING_TABLE_STATE_COUNT);
  ASSERT(event < RANGING_TABLE_EVENT_COUNT);
  EVENT_HANDLER[table->state][event](table);
}

/* Swarm Ranging */
static void processRangingMessage(Ranging_Message_With_Timestamp_t *rangingMessageWithTimestamp) {
  Ranging_Message_t *rangingMessage = &rangingMessageWithTimestamp->rangingMessage;
  uint16_t neighborAddress = rangingMessage->header.srcAddress;
  int neighborIndex = rangingTableSetSearchTable(&rangingTableSet, neighborAddress);

  /* Handle new neighbor */
  if (neighborIndex == -1) {
    Ranging_Table_t table;
    rangingTableInit(&table, neighborAddress);
    /* Ranging table set is full, ignore this ranging message. */
    if (!rangingTableSetAddTable(&rangingTableSet, table)) {
      DEBUG_PRINT("processRangingMessage: Ranging table is full = %d, cannot handle new neighbor %d.\n",
                  rangingTableSet.size,
                  neighborAddress);
      return;
    } else {
      neighborIndex = rangingTableSetSearchTable(&rangingTableSet, neighborAddress);
    }
  }

  Ranging_Table_t *neighborRangingTable = &rangingTableSet.tables[neighborIndex];
  /* Update Re */
  neighborRangingTable->Re.timestamp = rangingMessageWithTimestamp->rxTime;
  neighborRangingTable->Re.seqNumber = rangingMessage->header.msgSequence;
  /* Update latest received timestamp of this neighbor */
  neighborRangingTable->latestReceived = neighborRangingTable->Re;
  /* Update expiration time of this neighbor */
  neighborRangingTable->expirationTime = xTaskGetTickCount() + M2T(RANGING_TABLE_HOLD_TIME);

  /* Each ranging messages contains MAX_Tr_UNIT lastTxTimestamps, find corresponding
   * Tr according to Rr to get a valid Tr-Rr pair if possible, this approach may
   * help when experiencing continuous packet loss.
   */
  Ranging_Table_Tr_Rr_Buffer_t *neighborTrRrBuffer = &neighborRangingTable->TrRrBuffer;
  for (int i = 0; i < RANGING_MAX_Tr_UNIT; i++) {
    if (rangingMessage->header.lastTxTimestamps[i].timestamp.full
        && neighborTrRrBuffer->candidates[neighborTrRrBuffer->cur].Rr.timestamp.full
        && rangingMessage->header.lastTxTimestamps[i].seqNumber
            == neighborTrRrBuffer->candidates[neighborTrRrBuffer->cur].Rr.seqNumber) {
      rangingTableBufferUpdate(&neighborRangingTable->TrRrBuffer,
                               rangingMessage->header.lastTxTimestamps[i],
                               neighborTrRrBuffer->candidates[neighborTrRrBuffer->cur].Rr);
      break;
    }
  }
//  printRangingMessage(rangingMessage);

  /* Try to find corresponding Rf for MY_UWB_ADDRESS. */
  Timestamp_Tuple_t neighborRf = {.timestamp.full = 0, .seqNumber = 0};
  if (rangingMessage->header.filter & (1 << (uwbGetAddress() % 16))) {
    /* Retrieve body unit from received ranging message. */
    uint8_t bodyUnitCount = (rangingMessage->header.msgLength - sizeof(Ranging_Message_Header_t)) / sizeof(Body_Unit_t);
    for (int i = 0; i < bodyUnitCount; i++) {
      if (rangingMessage->bodyUnits[i].address == uwbGetAddress()) {
        neighborRf = rangingMessage->bodyUnits[i].timestamp;
        break;
      }
    }
  }

  /* Trigger event handler according to Rf */
  if (neighborRf.timestamp.full) {
    neighborRangingTable->Rf = neighborRf;
    rangingTableOnEvent(neighborRangingTable, RANGING_EVENT_RX_Rf);
  } else {
    rangingTableOnEvent(neighborRangingTable, RANGING_EVENT_RX_NO_Rf);
  }

  #ifdef ENABLE_DYNAMIC_RANGING_PERIOD
  /* update period according to distance and velocity */
  neighborRangingTable->period = M2T(DYNAMIC_RANGING_COEFFICIENT * (neighborRangingTable->distance / rangingMessage->header.velocity));
  /* bound ranging period between RANGING_PERIOD_MIN and RANGING_PERIOD_MAX */
  neighborRangingTable->period = MAX(neighborRangingTable->period, M2T(RANGING_PERIOD_MIN));
  neighborRangingTable->period = MIN(neighborRangingTable->period, M2T(RANGING_PERIOD_MAX));
  #endif
}

/* By default, we include each neighbor's latest rx timestamp to body unit in index order of ranging table, which
 * may cause ranging starvation, i.e. node 1 has many one-hop neighbors [2, 3, 4, 5, 6, 7, 8, 9, ..., 30], since
 * RANGING_MAX_BODY_UNIT < rangingTable.size, so each ranging message can only include a subset of it's one-hop
 * neighbor.
 * Say ranging table of node 1:
 *  index 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  ...
 *  id    2   7   5   3   8   11  15  6   16  17  23  24  25  26  4   18  ...
 * Then it is possible that ranging message N send by this node behaves like below (RANGING_MAX_BODY_UNIT = 4):
 *            ranging message 1: [2, 7, 5, 3]
 *            ranging message 2: [2, 7, 5, 3]
 *            ranging message 3: [2, 7, 5, 3]
 *            ranging message 4: [2, 7, 5, 3]
 *            ...
 * While the expected behavior is:
 *            ranging message 1: [2, 7, 5, 3]
 *            ranging message 2: [8, 11, 15, 6]
 *            ranging message 3: [16, 17, 23, 24]
 *            ranging message 4: [25, 26, 4, 18]
 *            ...
 * This behavior is lead by the fact that everytime we want to populate a new ranging message, the ranging tables
 * is traversed by index. If ENABLE_BUS_BOARDING_SCHEME, then the (table index, id) relationship is dynamically
 * changed according to the nextExpectedDeliveryTime of each table entry, which bypasses this issue implicitly.
 * Therefore, to solve this issue when BUS_BOARDING_SCHEME is not enabled, we should change the (table index, id)
 * relationship after populating new ranging message in the approach below:
 * Say the ranging tables of node 1:
 *  index 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  ...
 *  id    2   7   5   3   8   11  15  6   16  17  23  24  25  26  4   18  ...
 *            ranging message 1: [2, 7, 5, 3]
 * Then change the (index, id) relationship by moving the ranging table of included timestamp to last.
 *  index 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  ...
 *  id    11  15  6   16  17  23  24  25  26  4   18  2   7   5   3   8   ...
 *            ranging message 2: [11, 15, 6, 16]
 * Again:
 *  index 0   1   2   3   4   5   6   7   8   9   10  11  12  13  14  15  ...
 *  id    17  23  24  25  26  4   18  2   7   5   3   8   11  15  6   16  ...
 *            ranging message 3: [17, 23, 24, 25]
 * This makes the ranging table behaves like a cyclic array, the actual implementation have also considered the
 * nextExpectedDeliveryTime (only include timestamp with expected next delivery time less or equal than current
 * time) by sort the ranging table set by each timestamp's last send time.
 */
static Time_t generateRangingMessage(Ranging_Message_t *rangingMessage) {
  int8_t bodyUnitNumber = 0;
  rangingSeqNumber++;
  int curSeqNumber = rangingSeqNumber;
  rangingMessage->header.filter = 0;
  Time_t curTime = xTaskGetTickCount();
  /* Using the default RANGING_PERIOD when DYNAMIC_RANGING_PERIOD is not enabled. */
  Time_t taskDelay = M2T(RANGING_PERIOD);
  #ifdef ENABLE_BUS_BOARDING_SCHEME
  rangingTableSetRearrange(&rangingTableSet, COMPARE_BY_NEXT_EXPECTED_DELIVERY_TIME);
  #else
  rangingTableSetRearrange(&rangingTableSet, COMPARE_BY_LAST_SEND_TIME);
  #endif

  /* Generate message body */
  for (int index = 0; index < rangingTableSet.size; index++) {
    if (bodyUnitNumber >= RANGING_MAX_BODY_UNIT) {
      break;
    }
    Ranging_Table_t *table = &rangingTableSet.tables[index];
    if (table->latestReceived.timestamp.full) {
      /* Only include timestamps with expected delivery time less or equal than current time. */
      if (table->nextExpectedDeliveryTime > curTime) {
        continue;
      }
      table->nextExpectedDeliveryTime = curTime + M2T(table->period);
      table->lastSendTime = curTime;
      rangingMessage->bodyUnits[bodyUnitNumber].address = table->neighborAddress;
      /* It is possible that latestReceived is not the newest timestamp, because the newest may be in rxQueue
       * waiting to be handled.
       */
      rangingMessage->bodyUnits[bodyUnitNumber].timestamp = table->latestReceived;
      rangingMessage->header.filter |= 1 << (table->neighborAddress % 16);
      rangingTableOnEvent(table, RANGING_EVENT_TX_Tf);

      #ifdef ENABLE_DYNAMIC_RANGING_PERIOD
      /* Change task delay dynamically, may increase packet loss rate since ranging period now is determined
       * by the minimum expected delivery time.
       */
      taskDelay = MIN(taskDelay, table->nextExpectedDeliveryTime - curTime);
      /* Bound the dynamic task delay between RANGING_PERIOD_MIN and RANGING_PERIOD */
      taskDelay = MAX(RANGING_PERIOD_MIN, taskDelay);
      #endif

      #ifdef ROUTING_OLSR_ENABLE
      if (mprSetHas(getGlobalMPRSet(), table->neighborAddress)) {
        rangingMessage->bodyUnits[bodyUnitNumber].flags.MPR = true;
      } else {
        rangingMessage->bodyUnits[bodyUnitNumber].flags.MPR = false;
      }
      #endif

      bodyUnitNumber++;
    }
  }
  /* Generate message header */
  rangingMessage->header.srcAddress = MY_UWB_ADDRESS;
  rangingMessage->header.msgLength = sizeof(Ranging_Message_Header_t) + sizeof(Body_Unit_t) * bodyUnitNumber;
  rangingMessage->header.msgSequence = curSeqNumber;
  getLatestNTxTimestamps(rangingMessage->header.lastTxTimestamps, RANGING_MAX_Tr_UNIT);

  float velocityX, velocityY, velocityZ;
  adhocuwb_get_velocity(&velocityX, &velocityY, &velocityZ);

  velocity = sqrt(pow(velocityX, 2) + pow(velocityY, 2) + pow(velocityZ, 2));
  /* velocity in cm/s */
  rangingMessage->header.velocity = (short) (velocity * 100);
//  DEBUG_PRINT("generateRangingMessage: ranging message size = %u with %u body units.\n",
//              rangingMessage->header.msgLength,
//              bodyUnitNumber
//  );

  /* Keeps ranging table in order to perform binary search */
  rangingTableSetRearrange(&rangingTableSet, COMPARE_BY_ADDRESS);

  return taskDelay;
}

static void uwbRangingTxTask(void *parameters) {
  systemWaitStart();

  UWB_Packet_t txPacketCache;
  txPacketCache.header.srcAddress = uwbGetAddress();
  txPacketCache.header.destAddress = UWB_DEST_ANY;
  txPacketCache.header.type = UWB_RANGING_MESSAGE;
  txPacketCache.header.length = 0;
  Ranging_Message_t *rangingMessage = (Ranging_Message_t *) &txPacketCache.payload;

  while (true) {
    xSemaphoreTake(rangingTableSet.mu, portMAX_DELAY);
    xSemaphoreTake(neighborSet.mu, portMAX_DELAY);

    Time_t taskDelay = generateRangingMessage(rangingMessage);
    txPacketCache.header.length = sizeof(UWB_Packet_Header_t) + rangingMessage->header.msgLength;
    uwbSendPacketBlock(&txPacketCache);
//    printRangingTableSet(&rangingTableSet);
//    printNeighborSet(&neighborSet);

    xSemaphoreGive(neighborSet.mu);
    xSemaphoreGive(rangingTableSet.mu);
    vTaskDelay(taskDelay);
  }
}

static void uwbRangingRxTask(void *parameters) {
  systemWaitStart();

  Ranging_Message_With_Timestamp_t rxPacketCache;

  while (true) {
    if (xQueueReceive(rxQueue, &rxPacketCache, portMAX_DELAY)) {
      xSemaphoreTake(rangingTableSet.mu, portMAX_DELAY);
      xSemaphoreTake(neighborSet.mu, portMAX_DELAY);

      processRangingMessage(&rxPacketCache);
      topologySensing(&rxPacketCache.rangingMessage);

      xSemaphoreGive(neighborSet.mu);
      xSemaphoreGive(rangingTableSet.mu);
    }
    vTaskDelay(M2T(1));
  }
}

void rangingRxCallback(void *parameters) {
  // DEBUG_PRINT("rangingRxCallback \n");

  UWB_Packet_t *packet = (UWB_Packet_t *) parameters;

  dwTime_t rxTime;
  adhocuwb_readtxtimestamp((uint8_t *) &rxTime.raw);
  Ranging_Message_With_Timestamp_t rxMessageWithTimestamp;
  rxMessageWithTimestamp.rxTime = rxTime;
  Ranging_Message_t *rangingMessage = (Ranging_Message_t *) packet->payload;
  rxMessageWithTimestamp.rangingMessage = *rangingMessage;

  adhocuwb_xQueueSendFromISR(rxQueue, &rxMessageWithTimestamp);
}

void rangingTxCallback(void *parameters) {
  UWB_Packet_t *packet = (UWB_Packet_t *) parameters;
  Ranging_Message_t *rangingMessage = (Ranging_Message_t *) packet->payload;

  dwTime_t txTime;
  adhocuwb_readtxtimestamp((uint8_t *) &txTime.raw);

  Timestamp_Tuple_t timestamp = {.timestamp = txTime, .seqNumber = rangingMessage->header.msgSequence};
  updateTfBuffer(timestamp);
}

void rangingInit() {
  MY_UWB_ADDRESS = uwbGetAddress();
  rxQueue = xQueueCreate(RANGING_RX_QUEUE_SIZE, RANGING_RX_QUEUE_ITEM_SIZE);
  neighborSetInit(&neighborSet);
  neighborSetEvictionTimer = xTimerCreate("neighborSetEvictionTimer",
                                          M2T(NEIGHBOR_SET_HOLD_TIME / 2),
                                          pdTRUE,
                                          (void *) 0,
                                          neighborSetClearExpireTimerCallback);
  xTimerStart(neighborSetEvictionTimer, M2T(0));
  rangingTableSetInit(&rangingTableSet);
  rangingTableSetEvictionTimer = xTimerCreate("rangingTableSetEvictionTimer",
                                              M2T(RANGING_TABLE_HOLD_TIME / 2),
                                              pdTRUE,
                                              (void *) 0,
                                              rangingTableSetClearExpireTimerCallback);
  xTimerStart(rangingTableSetEvictionTimer, M2T(0));
  TfBufferMutex = xSemaphoreCreateMutex();

  listener.type = UWB_RANGING_MESSAGE;
  listener.rxQueue = NULL; // handle rxQueue in swarm_ranging.c instead of adhocdeck.c
  listener.rxCb = rangingRxCallback;
  listener.txCb = rangingTxCallback;
  uwbRegisterListener(&listener);

  adhocuwb_get_velocity_init();


  xTaskCreate(uwbRangingTxTask, ADHOC_UWB_RANGING_TX_TASK_NAME, UWB_TASK_STACK_SIZE, NULL,
              ADHOC_UWB_TASK_PRI, &uwbRangingTxTaskHandle);
  xTaskCreate(uwbRangingRxTask, ADHOC_UWB_RANGING_RX_TASK_NAME, UWB_TASK_STACK_SIZE, NULL,
              ADHOC_UWB_TASK_PRI, &uwbRangingRxTaskHandle);
}

LOG_GROUP_START(Ranging)
        LOG_ADD(LOG_INT16, distTo1, distanceTowards + 1)
        LOG_ADD(LOG_INT16, distTo2, distanceTowards + 2)
        LOG_ADD(LOG_INT16, distTo3, distanceTowards + 3)
        LOG_ADD(LOG_INT16, distTo4, distanceTowards + 4)
        LOG_ADD(LOG_INT16, distTo5, distanceTowards + 5)
        LOG_ADD(LOG_INT16, distTo6, distanceTowards + 6)
        LOG_ADD(LOG_INT16, distTo7, distanceTowards + 7)
        LOG_ADD(LOG_INT16, distTo8, distanceTowards + 8)
        LOG_ADD(LOG_INT16, distTo9, distanceTowards + 9)
        LOG_ADD(LOG_INT16, distTo10, distanceTowards + 10)
LOG_GROUP_STOP(Ranging)
