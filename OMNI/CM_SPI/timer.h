/* timer.h
 *
 * Timer used for timeout functions.
 */

#ifndef TIMER_H
#define TIMER_H

#define PHASE_1_LENGTH 100


#include "timer.h"
#include "nrf.h"

void init_timer(void);

void start_timeout(int millis);

#endif