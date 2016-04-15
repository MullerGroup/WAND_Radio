#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"
#include "timer.h"
#include "data_fifo.h"
#include "command_fifo.h"
#include "boards.h"
#include "spi_fifo.h"
#include "spi.h"
#include "nrf.h"
#include "nrf_gpio.h"
#include "app_error.h"

uint8_t *commands;
uint8_t length;
uint8_t i;
uint8_t *spi_packet;
bool success;
uint8_t j;
uint8_t c;

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    // Set LED2 high to indicate that error handler has been called.

    
    for (;;)
    {
        // No implementation needed.
    }
}

void init(void)
{
	/* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
    }

    const uint32_t err_code = spi_init();
    APP_ERROR_CHECK(err_code);

	radio_configure();
	init_timer();

	__enable_irq();
}

int main(void)
{
	init();
	radio_unpause_tx();
	start_timeout(PHASE_1_NORM);
	while (true)
	{
		// check command_fifo if there are commands to be sent
		commands = read_command();
		if (commands != 0)
		{
			success = false;
			length = (commands[0] & 0x7F)/5; // number of commands
			
			if (length != 0)
			{
				// there actually is at least one command
				if (length < get_capacity_spi_fifo())
				{
					// and the spi fifo can hold it
					// load the commands into the fifo
					c = 1;
					for (i=0;i<length;i++)
					{
						spi_packet = write_spi_fifo();
						if (spi_packet != 0)
						{
							spi_packet[129] = COMMAND_VALID;
							for (j=0;j<5;j++)
							{
								spi_packet[j+130] = commands[c + j];
							}
							finish_write_spi_fifo();
							c = c + 5;
						}
					}
					success = true;
				}
			}
			else
			{
				success = true;
			}

			if (success == true)
			{
				finish_read_command();
			}
			else
			{
				reset_read_command();
			}	
		}
	}
}


