#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include "debug_print.h"

static void debugPrintInitTask(void *argument);

osThreadId_t debugPrintInitTaskHandle;

const osThreadAttr_t debugPrintInitTask_attributes = {
  .name = "debugPrintInitTask",
  .stack_size = 128,
  .priority = (osPriority_t) osPriorityNormal,
};

void user_debugPrintInitTask_init(){
	debugPrintInitTaskHandle = osThreadNew(debugPrintInitTask, NULL, &debugPrintInitTask_attributes);
}

static const UserInit debugprint_init = {
		.init = user_debugPrintInitTask_init,
};

USER_INIT(debugprint_init);

static void debugPrintInitTask(void *argument)
{
	debug_print_init();
	printf("Hello World!Debug print init ok!\n");
}
