#ifndef ADC_H
#define ADC_H

#include <avr/io.h>
#include <avr/interrupt.h>

int ADC_Init();

void ADC_Task(uint8_t *measuredValue);

#endif // ADC_H
