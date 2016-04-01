#include "ADC.h"
#include "main.h"
#include "usart.h"

uint8_t ADC_result[3] = {0, 0, 0};
uint32_t tempValue = 0;
uint8_t tempCounter = 0;

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
/*
	ADMUX = 0x00; // Подключен канал ADC0
	ADMUX |= _BV(ADLAR) // Выравнивание результата по левой границе
			| _BV(REFS0); // Опорное напряжение с AVcc
	/*
		Благодаря выравниванию регистры результата будут выглядеть следующим образом:

	ADCH: 7  6  5  4  3  2  1  0 ADCL: 7  6  5  4  3  2  1  0
		 [9][8][7][6][5][4][3][2]	  [1][0][x][x][x][x][x][x]

		Два младших бита с шумом оказываются в регистре ADCL, что позволяет читать
		результат из ADCH без сдвига на 2
	*//*
    ADCSR = _BV(ADEN) // Разрешение АЦП
          | _BV(ADFR) // Режим непрерывного преобразования
          | _BV(ADSC) // Запуск преобразования
          | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) // Предделитель на 128
          | _BV(ADIE); // Разрешение прерывания от АЦП

    return 0;*/
}


ISR(ADC_vect)
{
    ADCSR &= ~_BV(ADSC); // Остановить преобразование

    uint8_t MUX_value = ADMUX & 0x03; // Считать номер текущего канала

	// Получить результат преобразования
    tempValue += (ADCW>>2);
    tempCounter ++;

    if (tempCounter == 255)
    {
		// Посчитать среднее значение
		/*
			Счетчик tempCounter содержит количество сложений результатов.
			Чтобы найти среднее, надо поделить именно на него, потому что
			когда счетчик был равен 0, в tempValue тоже был 0.
		*/
        ADC_result[MUX_value] = tempValue / tempCounter;

        tempCounter = 0;
        tempValue = 0;

		// Переключить канал АЦП
		if (MUX_value == 2)
		{
			MUX_value = 0;
			// Передать результат на обработку
			ADC_Task(&ADC_result[0]);
		}
		else
			MUX_value++;
		ADMUX = MUX_value & 0x03;
    }

    ADCSR |= _BV(ADSC); // Запустить преобразование
}


