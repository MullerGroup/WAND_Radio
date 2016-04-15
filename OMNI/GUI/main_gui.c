#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"
#include "uart.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "nrf_gpio.h"
#include "boards.h"

uint8_t *data;		// pointer to buffer with data to be transmitted over UART
uint8_t length;		// number of bytes of data to be transmitted in buffer

uint32_t uart_bytes;	// cumulative number of bytes transmitted over UART

void init(void)
{
    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
    }

    uart_bytes = 0;

	simple_uart_config(RTS_PIN_NUMBER, TX_PIN_NUMBER, CTS_PIN_NUMBER, RX_PIN_NUMBER, HWFC);
	radio_configure();
}

int main(void)
{
	init();

	while (true)
	{
		// check data fifo for data to transmit over UART
		
		if (get_num_data() > 5)
		{
			// there was data to be sent over UART
			data = read_data();
			length = data[1];			// how many valid bytes in packet
			simple_uart_putbuf(data + 2, length);
			finish_read_data();
			uart_bytes = uart_bytes + length;
		}
	}
}