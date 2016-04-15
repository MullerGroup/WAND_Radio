#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"
#include "spi.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "nrf_gpio.h"
#include "boards.h"

uint8_t *data;		// pointer to buffer with data to be transmitted over UART
uint8_t length;		// number of bytes of data to be transmitted in buffer

uint32_t uart_bytes;	// cumulative number of bytes transmitted over UART
uint8_t command[COMMAND_SIZE];
uint8_t *write_pointer;
uint8_t i;

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

	spi_init();
	radio_configure();
}


int main(void)
{
	init();

	while (true)
	{
		// check data fifo for data to transmit over UART
		
		if (get_num_data() > 0)
		{
			// there was data to be sent over UART
			data = read_data();
			length = data[1];			// how many valid bytes in packet
			if (length > 0)
			{
				spi_write(data + 2, length);
				uart_bytes = uart_bytes + length;
			}	
			finish_read_data();
		}
		if (get_read_flag())
		{
			while (spi_read_with_NAK(command, COMMAND_SIZE))
			{
				write_pointer = write_command();
				if (write_pointer != 0)
				{
					for (i=0;i<COMMAND_SIZE;i++)
					{
						write_pointer[i] = command[i];
					}
					finish_write_command();
				}
				clear_read_flag();
			}
		}
	}
}