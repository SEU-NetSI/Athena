/*
 * tmux1574.h
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#ifndef BSP_COMPONENTS_TMUX1574_TMUX1574_H_
#define BSP_COMPONENTS_TMUX1574_TMUX1574_H_

#include "stm32h7xx_ll_gpio.h"
#include "main.h"
#include "FreeRTOS.h"

#define TMUX_EN_Pin LL_GPIO_PIN_11
#define TMUX_EN_Port GPIOB
#define TMUX_SEL_Pin LL_GPIO_PIN_7
#define TMUX_SEL_Port GPIOC

void EnableTmux();

void DisableTmux();

void EnableChannelA();

void EnableChannelB();

#endif /* BSP_COMPONENTS_TMUX1574_TMUX1574_H_ */
