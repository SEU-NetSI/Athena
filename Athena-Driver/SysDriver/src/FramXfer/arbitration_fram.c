/*
 * arbitration_fram.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#include "arbitration_fram.h"

SemaphoreHandle_t FramSwitchMutex = NULL;

void FramArbitrationInit() {
    FramSwitchMutex = xSemaphoreCreateMutex();
    if (FramSwitchMutex == NULL) {
        Error_Handler();
    }
}
