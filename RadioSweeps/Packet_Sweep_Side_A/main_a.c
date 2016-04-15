#include <stdbool.h>
#include "uart.h"
#include "radio_config.h"
#include "nrf_gpio.h"
#include "boards.h"

void init(void)
{
	/* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
    }

    simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
    radio_configure(DEFAULT_PACKET_SIZE);
}

int main(void)
{
	init();
	while (true)
	{
	}
}