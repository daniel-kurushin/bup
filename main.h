#ifndef MAIN_H
#define MAIN_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "pindefines.h"
#include "usart.h"
#include "timer.h"
#include "ADC.h"

#include "build.h"

typedef enum
{
    ADC_48V_c = 0,
    ADC_12V_c = 1,
    ADC_12V_u = 2
} ADC_CHANNELS_t;

typedef enum
{
    BEEP_OK    = 0x0,
    BEEP_ERROR = 0xff
} BEEPTYPES;

typedef enum
{
    NONE = 0b00000000,
    SELF = 0b00000011,
    P12  = 0b00001100,
    P48  = 0b00110000
} POWERTYPES;

int initPINS();

void beep(BEEPTYPES bt);
void power_on(uint8_t channels);
void power_off(uint8_t channels);

#endif
