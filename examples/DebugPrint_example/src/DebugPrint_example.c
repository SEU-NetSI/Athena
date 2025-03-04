#include "DebugPrint_example.h"
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

osThreadId_t DebugTaskHandle;

const osThreadAttr_t debugTask_attributes = {
  .name = "DebugTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_debugtask_init(){
	DebugTaskHandle = osThreadNew(DebugTask, NULL, &debugTask_attributes);
}

static const UserInit debug_init = {
		.init = user_debugtask_init
};

static void DebugTask(void *argument)
{

  while(1)
  {
    DEBUG_PRINTF("this is a test: %u \n", 85);
//    test print float
    vTaskDelay(1);
    DEBUG_PRINTF("this is a test: %.2f \n", 5.82);
    //test print int
    vTaskDelay(1);
    DEBUG_PRINTF("this is a test: %i \n", -19);
    //test print string
    DEBUG_PRINTF("this is a test: %s \n","hello world");
	vTaskDelay(1);
  }
}
USER_INIT(debug_init);
