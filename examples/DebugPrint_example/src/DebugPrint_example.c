#include "DebugPrint_example.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

static void DebugTask(void *argument);
static void DebugTask_new(void *argument);

osThreadId_t DebugTaskHandle;
osThreadId_t DebugTaskHandle_new;

const osThreadAttr_t debugTask_attributes = {
  .name = "DebugTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t debugTask_new_attributes = {
  .name = "DebugTask_new",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_debugtask_init(){
	DebugTaskHandle = osThreadNew(DebugTask, NULL, &debugTask_attributes);
	DebugTaskHandle_new = osThreadNew(DebugTask_new, NULL, &debugTask_new_attributes);
}

static const UserInit debug_init = {
		.init = user_debugtask_init
};

static void DebugTask(void *argument)
{

  while(1)
  {
    DEBUG_PRINTF("this is a test: %u \n", 85);
    //test print float
//    DEBUG_PRINTF("this is a test: %.2f \n", 5.82);
//    //test print int
//    DEBUG_PRINTF("this is a test: %i \n", -19);
//    //test print string
//    DEBUG_PRINTF("this is a test: %s \n","hello world");
	vTaskDelay(2000);
  }
}


static void DebugTask_new(void *argument)
{

  while(1)
  {
    DEBUG_PRINTF("dao luan is a test: %u \n", 70);
	vTaskDelay(20);
  }
}

void Debug_example(void *arugment)
{
    while(1)
    {
        vTaskDelay(2000);
        //test print uint 
        DEBUG_PRINTF("this is a test: %u \n", 85);
        //test print float
        DEBUG_PRINTF("this is a test: %.2f \n", 5.82);
        //test print int
        DEBUG_PRINTF("this is a test: %i \n", -19);
        //test print string
        DEBUG_PRINTF("this is a test: %s \n","hello world");
    }

}

//USER_INIT(debug_init);
