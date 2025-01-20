/*
 * tmux1574.h
 *
 *  Created on: Jan 20, 2025
 *      Author: mfxjx
 */

#ifndef BSP_COMPONENTS_TMUX1574_TMUX1574_H_
#define BSP_COMPONENTS_TMUX1574_TMUX1574_H_

#include "stm32l4xx_ll_gpio.h"
#include "main.h"

void EnableTmux();

void DisableTmux();

void EnableChannelA();

void EnableChannelB();

#endif /* BSP_COMPONENTS_TMUX1574_TMUX1574_H_ */
