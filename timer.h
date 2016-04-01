#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

int systemTimerInit();

void systemTimerTask();

#endif
