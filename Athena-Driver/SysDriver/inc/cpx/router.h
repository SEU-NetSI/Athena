/*
 * router.h
 *
 *  Created on: Dec 30, 2024
 *      Author: 18387
 */

#ifndef SYSDRIVER_INC_CPX_ROUTER_H_
#define SYSDRIVER_INC_CPX_ROUTER_H_

#include <stdint.h>
#include <stddef.h>

void routerInit(void);
void cpxInternalRouterReceiveOthers(CPXPacket_t * packet);
void cpxSendPacketBlocking(const CPXPacket_t * packet);
#endif /* SYSDRIVER_INC_CPX_ROUTER_H_ */
