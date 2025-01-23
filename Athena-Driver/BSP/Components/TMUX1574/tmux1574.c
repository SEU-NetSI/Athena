/*
 * tmux1574.c
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */
#include "tmux1574.h"
#include "arbitration_fram.h"

void EnableTmux(){
	LL_GPIO_ResetOutputPin(TMUX_EN_Port, TMUX_EN_Pin);
}

void DisableTmux(){
	LL_GPIO_SetOutputPin(TMUX_EN_Port, TMUX_EN_Pin);
}

void EnableChannelA(){
	xSemaphoreTake(FramSwitchMutex, portMAX_DELAY);
	LL_GPIO_ResetOutputPin(TMUX_SEL_Port, TMUX_SEL_Pin);
	xSemaphoreGive(FramSwitchMutex);
}

void EnableChannelB(){
	xSemaphoreTake(FramSwitchMutex, portMAX_DELAY);
	LL_GPIO_SetOutputPin(TMUX_SEL_Port, TMUX_SEL_Pin);
	xSemaphoreGive(FramSwitchMutex);
}

