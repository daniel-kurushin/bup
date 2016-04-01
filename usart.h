#ifndef USART_H
#define USART_H

#include <avr/io.h>
#include <stdint.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <stdlib.h>
//#include <util/delay.h>
#include <string.h>

#define SYNCHRO_BYTE					0xAE

#define PACKET_SYNCHRO_OFFSET			0
#define PACKET_TARGET_ADDRESS_OFFSET	2
#define PACKET_SOURCE_ADDRESS_OFFSET	3
#define PACKET_TYPE_OFFSET				4
#define PACKET_LENGTH_OFFSET			5
#define PACKET_CRC_OFFSET				6
#define PACKET_DATA_OFFSET				7

#define PACKET_HEADER_LENGTH			7
#define PACKET_MAX_LENGTH				50

#define DEVICE_TYPE                     0xBB
#define DEVICE_ADDR                     0xBB

typedef enum
{
    RCVR_READY,
    RCVR_SYNCHRO_FIRSTBYTE_READY,
    RCVR_SYNCHRO_READY,
    RCVR_ADDR_READY,
    RCVR_LENGTH_READY
} RCVR_STATE_t;

int USART_Init();
void USART_enableTransmit(bool enable);
void USART_Transmit(uint8_t ch);
uint8_t getCRC(unsigned char *packet, uint8_t packetLen);
void commandHandler(unsigned char *pPacketData, uint8_t nPacketLength);

void transmitPacket(uint8_t nPacketType, char *pPacketData, uint8_t nPacketLength);

#endif
