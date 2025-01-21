/*
 * tmux1574.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */
#include "tmux1574.h"

void EnableTmux(){
	LL_GPIO_ResetOutputPin(TMUX_EN_Port, TMUX_EN_Pin);
}

void DisableTmux(){
	LL_GPIO_SetOutputPin(TMUX_EN_Port, TMUX_EN_Pin);
}

void EnableChannelA(){
	xSemaphoreTake(FramSwtichMutex, portMAX_DELAY);
	LL_GPIO_ResetOutputPin(TMUX_SEL_Port, TMUX_SEL_Pin);
	xSemaphoreGive(FramSwtichMutex);
}

void EnableChannelB(){
	xSemaphoreTake(FramSwtichMutex, portMAX_DELAY);
	LL_GPIO_SetOutputPin(TMUX_SEL_Port, TMUX_SEL_Pin);
	xSemaphoreGive(FramSwtichMutex);
}

