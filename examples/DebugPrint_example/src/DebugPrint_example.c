#include "DebugPrint_example.h"
void Debug_example(void *arugment)
{
    while(1)
    {
        vTaskDelay(5000);
        //test print uint 
        DEBUG_PRINTF("this is a test: %u \n", 85);
        //test print float
        vTaskDelay(5000);
        DEBUG_PRINTF("this is a test: %.2f \n", 5.82);
        //test print int
        vTaskDelay(5000);
        DEBUG_PRINTF("this is a test: %i \n", -19);
        vTaskDelay(5000);
        //test print string
        DEBUG_PRINTF("this is a test: %s \n","hello world");
    }

}
