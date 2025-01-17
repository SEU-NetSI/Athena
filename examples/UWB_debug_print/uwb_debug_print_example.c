#include "uwb_send_print.h"
#include "main.h"
#include "syslaunch.h"

static osThreadId_t uwbDebugPrintTaskHandle;

const osThreadAttr_t uwb_debug_example_taskAttributes = {
	  .name = "uwb_debug_example_task",
	  .stack_size =  2 * UWB_FRAME_LEN_MAX * sizeof(StackType_t), //TODO: check whether this works
	  .priority = (osPriority_t) osPriorityNormal,
};

static void uwb_debug_example_task(void *arugment)
{
	while(!getInitStatus()){
	    	vTaskDelay(10);
	    }
    while(1)
    {
        vTaskDelay(100);
        //test print uint
        UWB_DEBUG_PRINTF("this is a test: %u \n", 85);
        //test print float
        UWB_DEBUG_PRINTF("this is a test: %.2f \n", 5.82);
        //test print int
        UWB_DEBUG_PRINTF("this is a test: %i \n", -19);
        //test print string
        UWB_DEBUG_PRINTF("this is a test: %s \n","hello world");
    }
}

static void uwb_debug_print_init(){
	uwbDebugPrintTaskHandle = osThreadNew(uwb_debug_example_task, NULL, &uwb_debug_example_taskAttributes);
}

static const UserInit uwb_debug_print_init_struct = {
		.init = uwb_debug_print_init,
};

//USER_INIT(uwb_debug_print_init_struct);
