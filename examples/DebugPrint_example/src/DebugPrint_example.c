#include "DebugPrint_example.h"
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
        DEBUG_PRINTF("this is a teset: %s \n","hello world");
    }

}
