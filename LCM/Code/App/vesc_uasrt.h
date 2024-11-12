#ifndef __VESC_USART_H
#define __VESC_USART_H

#include "datatypes.h"
#include "lcm.h"

extern uint8_t VESC_RX_Buff[80];
extern uint8_t VESC_RX_Flag;
extern dataPackage data;
extern lcmConfig_t lcmConfig;
extern uint8_t errCode;

void Get_Vesc_Pack_Data(COMM_PACKET_ID id);
uint8_t Protocol_Parse(uint8_t * message);

#endif


