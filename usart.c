#include "usart.h"

RCVR_STATE_t receiverState;

unsigned char *rxBuffer;
uint8_t rxBufferDataLen;

unsigned char *txBuffer;
uint8_t txBufferDataLen;
uint8_t txBufferDataPos;

uint8_t getCRC(unsigned char *packet, uint8_t packetLen)
{
    uint8_t summ = 0;
    for (uint8_t i = 0; i < packetLen; i++)
    {
        summ += packet[i];
    }
    summ -= packet[PACKET_CRC_OFFSET];

    return summ;
}


int USART_Init()
{
    int result = 0;

    UBRRH = (51 >> 8);
    UBRRL = (51 & 0xFF);
    // Удвоение скорости отключено
    UCSRA = 0;
    //Разрешение на прием и на передачу через USART, разрешение прерываний RX*
    UCSRB = _BV(RXEN) | _BV(TXEN) | _BV(RXCIE);
    //Формат кадра: 8 бит данных, 1 стоп-бит.
    UCSRC = _BV(URSEL) | _BV(UCSZ0) | _BV(UCSZ1);

    txBuffer = (unsigned char *)malloc(255);
    txBufferDataLen = 0;
    txBufferDataPos = 0;

    rxBuffer = (unsigned char *)malloc(255);
    rxBufferDataLen = 0;

    if ((txBuffer == NULL) | (rxBuffer == NULL))
        result = 1;

    return result;
}

void USART_enableTransmit(bool enable)
{
	if (enable)
		UCSRB |= _BV(UDRIE);
	else
		UCSRB &= ~_BV(UDRIE);
}

void USART_Transmit(uint8_t ch)
{
	while (!(UCSRA & _BV(UDRE))); //Ожидание опустошения буфера передачи
	UDR = ch; //Начало передачи данных
}

void transmitPacket(uint8_t nPacketType, char *pPacketData, uint8_t nPacketLength)
{
    unsigned char *answerPacket = (unsigned char *)malloc(PACKET_HEADER_LENGTH + nPacketLength); // Создаем пакет длиной 8 байт

    answerPacket[PACKET_SYNCHRO_OFFSET] = SYNCHRO_BYTE;
    answerPacket[PACKET_SYNCHRO_OFFSET+1] = SYNCHRO_BYTE;
    answerPacket[PACKET_TARGET_ADDRESS_OFFSET] = 0x00;
    answerPacket[PACKET_SOURCE_ADDRESS_OFFSET] = DEVICE_ADDR;
    answerPacket[PACKET_TYPE_OFFSET] = nPacketType;
    answerPacket[PACKET_LENGTH_OFFSET] = nPacketLength + PACKET_HEADER_LENGTH;

    if (pPacketData != NULL)
        memcpy((void *)&answerPacket[PACKET_DATA_OFFSET], (void *)pPacketData, nPacketLength);

    answerPacket[PACKET_CRC_OFFSET] = getCRC(answerPacket, PACKET_HEADER_LENGTH + nPacketLength);

    // Скопировать пакет в буфер передачи
    memcpy(txBuffer, answerPacket, PACKET_HEADER_LENGTH + nPacketLength);
    txBufferDataLen = PACKET_HEADER_LENGTH + nPacketLength;
    txBufferDataPos = 0;

    // Инициировать отправку
    USART_enableTransmit(true);
    // Освободить выделенную под пакет память
    free(answerPacket);
}

// Прерывание по окончанию приема байта модулем USART
ISR(USART_RXC_vect)
{
	unsigned char ch = UDR;

	switch (receiverState)
    {
    case RCVR_READY:
        // Приемник готов. Ждем первый синхро-байт.
        if (ch == SYNCHRO_BYTE)
        {
            // Если байт похож на синхро-байт, то добавляем его в буфер
            // и изменяем состояние автомата
            rxBuffer[rxBufferDataLen] = ch;
            rxBufferDataLen++;
            receiverState = RCVR_SYNCHRO_FIRSTBYTE_READY;
        }
        break;
    case RCVR_SYNCHRO_FIRSTBYTE_READY:
        // Первый синхро-байт принят. Ждем второй синхро-байт.
        if (ch == SYNCHRO_BYTE)
        {
            // Если байт похож на синхро-байт, то добавляем его в буфер
            // и изменяем состояние автомата
            rxBuffer[rxBufferDataLen] = ch;
            rxBufferDataLen++;
            receiverState = RCVR_SYNCHRO_READY;
        }
        else
        {
            // иначе очищаем буфер и ставим состояние "Готов".
            rxBufferDataLen = 0;
            receiverState = RCVR_READY;
        }
        break;
    case RCVR_SYNCHRO_READY:
        // Оба синхро-байта приняты. Ждем адрес пакета.
        if (ch == 0xbb)
        {
            // Если адрес пакета равен адресу устройства, то добавляем байт в буфер
            // и изменяем состояние автомата
            rxBuffer[rxBufferDataLen] = ch;
            rxBufferDataLen++;
            receiverState = RCVR_ADDR_READY;
        }
        else
        {
            // иначе очищаем буфер и ставим состояние "Готов".
            rxBufferDataLen = 0;
            receiverState = RCVR_READY;
        }
        break;
    case RCVR_ADDR_READY:
        // Адрес принят и подтвержден. Ждем длину пакета.
        rxBuffer[rxBufferDataLen] = ch;
        rxBufferDataLen++;
        if (rxBufferDataLen > PACKET_LENGTH_OFFSET)
        {
            uint8_t packetLength = rxBuffer[PACKET_LENGTH_OFFSET];
            // Если длина пакета принята, то проверяем ее валидность
            if ((packetLength >= PACKET_HEADER_LENGTH)
                    & (packetLength <= PACKET_MAX_LENGTH))
            {
                // Если длина валидна, то изменяем состояние автомата
                receiverState = RCVR_LENGTH_READY;
            }
            else
            {
                // иначе очищаем буфер и ставим состояние "Готов".
                rxBufferDataLen = 0;
                receiverState = RCVR_READY;
            }
        }
        break;
    case RCVR_LENGTH_READY:
        // Длина пакета принята. Ждем приема всех байт пакета.
        rxBuffer[rxBufferDataLen] = ch;
        rxBufferDataLen++;
        if (rxBufferDataLen == rxBuffer[PACKET_LENGTH_OFFSET])
        {
            // Если пакет принят целиком, то передаем его на обработку,
            // очищаем буфер и ставим состояние "Готов"
            unsigned char *buffer = (unsigned char *)malloc(rxBufferDataLen);
            memcpy(buffer, rxBuffer, rxBufferDataLen);
			commandHandler(buffer, rxBufferDataLen);
            rxBufferDataLen = 0;
            receiverState = RCVR_READY;
        }
        break;
    }
}

// Прерывание по опустошению буфера передачи USART
ISR(USART_UDRE_vect)
{
	// Если еще есть данные для передачи, то
	if (txBufferDataPos < txBufferDataLen)
	{
		// Скармливаем очередной байт буферу передачи
		UDR = txBuffer[txBufferDataPos];
		// Увеличиваем текущую позицию в буфере
		txBufferDataPos++;
	}
	else
		// Иначе гасим прерывания на опустошение буфера передачи
		USART_enableTransmit(false);
}
//---------------------------------------------------------------------------//
