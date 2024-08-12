#include "adhocuwb_impl.h"


#define CS_PIN DECK_GPIO_IO1

#define DEFAULT_RX_TIMEOUT 0xFFFFF

void adhocuwb_get_velocity_init(){
	return;
}

void adhocuwb_get_velocity(float* velocityX, float* velocityY, float* velocityZ){
	*velocityX = 1.1;
	*velocityY = 22.22;
	*velocityZ = 333.333;
}

BaseType_t adhocuwb_xQueueSendFromISR(
    QueueHandle_t xQueue,
    const void * pvItemToQueue)
{
	BaseType_t retval;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;

	// Send data to the queue
	retval = xQueueSendFromISR(xQueue, pvItemToQueue, &xHigherPriorityTaskWoken);

	// Check if a higher priority task was woken and request a context switch if so
	// This macro requests a context switch to the higher priority task
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
	return retval;
}

void adhocuwb_vTaskNotifyGiveFromISR(TaskHandle_t taskHandle) {
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	vTaskNotifyGiveFromISR(taskHandle, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
