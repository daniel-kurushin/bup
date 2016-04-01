#include "ADC.h"
#include "main.h"
#include "usart.h"

uint8_t ADC_result[3] = {0, 0, 0};
uint32_t valueTemps[3] = {0, 0, 0};
uint8_t counters[3] = {0, 0, 0};

int ADC_Init()
{
    ADMUX = 0x01; // Подключаем канал ADC0, внешний ИОН на AREF
    //ADMUX |= _BV(REFS0) | _BV(REFS1); // внутренний ИОН на 2,56 вольт
    ADCSRA |= _BV(ADEN) // разрешение АЦП
             | _BV(ADFR) // режим непрерывного преобразования
             | _BV(ADSC) // запуск преобразования
             | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) // предделитель на 128
             | _BV(ADIE); // разрешение прерывания от АЦП

    return 0;
}


ISR(ADC_vect)
{
    //ADCSRA &= ~_BV(ADSC);

    uint8_t MUX_value = ADMUX & 0x03;

    ADC_result[0] = ADC;
    ADC_result[1] = ADMUX;
    ADC_result[2] = ADCW;
    ADC_Task(&ADC_result[0]);
    //USART_Transmit(ADCW >> 2);
    /*valueTemps[MUX_value] += (uint8_t)(ADCW >> 2);
    counters[MUX_value] ++;

    if (counters[MUX_value] == 255)
    {
        uint16_t buf = valueTemps[MUX_value] / 256;
        if (buf <= 256)
            ADC_result[MUX_value] = (uint8_t)buf;
        else
            beep(BEEP_ERROR);
        counters[MUX_value] = 0;
        valueTemps[MUX_value] = 0;
        ADC_Task(&ADC_result[0]);
    }*/

    //measuredValues[MUX_value] = ADCW >> 2;
    //ADC_Task(measuredValues[MUX_value]);

    if (MUX_value == 2)
        MUX_value = 0;
    else
        MUX_value++;
    ADMUX = MUX_value & 0x03;

    //ADCSRA |= _BV(ADSC);
}


