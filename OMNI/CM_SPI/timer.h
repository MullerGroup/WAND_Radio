/* timer.h
 *
 * Timer used for timeout functions.
 */

#ifndef TIMER_H
#define TIMER_H

#define PHASE_1_LENGTH 10
#define PHASE_1_STREAM 1000


#include "timer.h"
#include "nrf.h"

void init_timer(void);

void start_timeout(int millis);

#endif
