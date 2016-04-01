/*
 */

#include "main.h"

uint8_t systemTimerCounter = 0;
uint8_t ADC_values[3];


int initPINS()
{
    DIR_power |= _BV(PIN_power_self) |
                 _BV (PIN_power_12) |
                 _BV(PIN_power_48);
    DIR_speaker |= _BV(PIN_speaker);


    DIR_ADC &= ~_BV(ADC_CH0) & ~_BV(ADC_CH1) & ~_BV(ADC_CH2);
    PORT_ADC &= ~_BV(ADC_CH0) & ~_BV(ADC_CH1) & ~_BV(ADC_CH2);

    return 0;
}

void dot()
{
    PORT_speaker |= _BV(PIN_speaker);
    _delay_ms(300);
    PORT_speaker &= ~_BV(PIN_speaker);
    _delay_ms(300);
}

void dash()
{
    PORT_speaker |= _BV(PIN_speaker);
    _delay_ms(1000);
    PORT_speaker &= ~_BV(PIN_speaker);
    _delay_ms(300);
}

void beep(BEEPTYPES bt)
{
    switch (bt)
    {
    case BEEP_OK:
    {
        dot();
        break;
    }
    case BEEP_ERROR:
    {
        dot();
        dot();
        dot();
        dash();
        dash();
        dash();
        dot();
        dot();
        dot();
    }
    }
}
uint8_t * getPowerChStatus()
{
    static uint8_t st [3];
    st[0] = (PORT_power & _BV(PIN_power_self)) ? 0xff : 0x00;
    st[1] = (PORT_power & _BV(PIN_power_12))   ? 0xff : 0x00;
    st[2] = (PORT_power & _BV(PIN_power_48))   ? 0xff : 0x00;
    return st;
}

void power_on(uint8_t channels)
{
    if(channels & SELF) PORT_power |= _BV(PIN_power_self);
    if(channels & P12)  PORT_power |= _BV(PIN_power_12);
    if(channels & P48)  PORT_power |= _BV(PIN_power_48);
}

void power_off(uint8_t channels)
{
    if(channels & SELF) PORT_power &= ~_BV(PIN_power_self);
    if(channels & P12)  PORT_power &= ~_BV(PIN_power_12);
    if(channels & P48)  PORT_power &= ~_BV(PIN_power_48);
}

/*
    This function was declared in USART.h
*/
void commandHandler(unsigned char *pPacketData, uint8_t nPacketLength)
{
    switch (pPacketData[PACKET_TYPE_OFFSET])
    {
    case  0x00:
    {
        // Обработка для всех устройств одинаковая
        // Получен запрос типа устройства.
        char cPacketData = DEVICE_TYPE;
        transmitPacket(0x00, &cPacketData, sizeof(char));
        // Выход из функции
        break;
    }
    case 0x01:
    {
        // Enable output 1
        power_on(P12);
        uint8_t *cPacketData = getPowerChStatus();
        transmitPacket(0x01, (char *)cPacketData, 3);
        break;
    }
    case 0x10:
    {
        // Disable output 1
        power_off(P12);
        uint8_t *cPacketData = getPowerChStatus();
        transmitPacket(0x10, (char *)cPacketData, 3);
        break;
    }
    case 0x02:
    {
        // Enable output 2
        power_on(P48);
        uint8_t *cPacketData = getPowerChStatus();
        transmitPacket(0x02, (char *)cPacketData, 3);
        break;
    }
    case 0x20:
    {
        // Disable output 2
        power_off(P48);
        uint8_t *cPacketData = getPowerChStatus();
        transmitPacket(0x20, (char *)cPacketData, 3);
        break;
    }
    case 0x04:
    {
        transmitPacket(0x04, (char *)&ADC_values[0], 3);
        break;
    }
    case 0xff:
    {
        // Suicide
        power_off(SELF);
        break;
    }
    }

    // Освободить выделенную под пакет память
    free(pPacketData);
}

/*
    This function was declared in TIMER.h
*/
void systemTimerTask()
{
    if (systemTimerCounter < 122)
        systemTimerCounter++;
    else
    {
        systemTimerCounter = 0;
    }
}

/*
    This function was declared in ADC.h
*/
void ADC_Task(uint8_t *measuredValue)
{
    for (uint8_t i = 0; i < 3; i++)
        ADC_values[i] = measuredValue[i];
}

int main(void)
{

    if(!initPINS() &&
            !ADC_Init() &&
            !USART_Init() &&
            !systemTimerInit())
    {
        _delay_ms(500);
        power_on(SELF);
        beep(BEEP_OK);
        power_on(P12);
        power_on(P48);
    }
    else
    {
        while(1) beep(BEEP_ERROR);
    }

    sei();

    while(1);
}
