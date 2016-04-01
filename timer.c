#include "timer.h"

int systemTimerInit()
{
    TCCR0 = _BV(CS02); // Режим Normal, делитель на 256
    TIMSK |= _BV(TOIE0); // Разрешить прерывание по переполнению Т0

    return 0;
}

ISR(TIMER0_OVF_vect)
{
    systemTimerTask();
}
