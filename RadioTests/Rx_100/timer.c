#include "timer.h"
#include "nrf.h"

void init_timer(void)
{
	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;
	NRF_TIMER0->PRESCALER = 0;
	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->TASKS_START = 1;
}

uint32_t elapsed_time(void)
{
	uint32_t elapsed;

	NRF_TIMER0->TASKS_CAPTURE[0] = 1;
	NRF_TIMER0->TASKS_CLEAR = 1;
	elapsed = NRF_TIMER0->CC[0];
	return elapsed;
}
