/*
 * fram_sys_xfer.h
 *
 *  Created on: Jan 21, 2025
 *      Author: mfxjx
 */

#ifndef SYSDRIVER_INC_FRAMXFER_FRAM_SYS_XFER_H_
#define SYSDRIVER_INC_FRAMXFER_FRAM_SYS_XFER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct {
    uint16_t length;
    uint8_t type;
    uint8_t* content;
} DataPacket;

void FramXfertoPerform(uint8_t* data, size_t len);
void XfertoPerformance(DataPacket* packet);
#endif /* SYSDRIVER_INC_FRAMXFER_FRAM_SYS_XFER_H_ */
