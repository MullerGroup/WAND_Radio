#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"
#include "spi.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_delay.h"

uint8_t *data;		// pointer to buffer with data to be transmitted over UART
uint8_t length;		// number of bytes of data to be transmitted in buffer

uint8_t command[COMMAND_SIZE];
uint8_t *write_pointer;
uint8_t i;
uint32_t bad_lengths = 0;
uint32_t aa_count = 0;
uint32_t spi_bytes = 0;
uint32_t data_fifo_bytes_read = 0;

void init(void)
{
    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
    }

	spi_init();
	radio_configure();
}


int main(void)
{
	init();

	while (true)
	{
		// check data fifo for data to transmit over SPI
        __disable_irq();         
        data = read_data();
        __enable_irq(); 

        if (data != 0)
        {
			length = data[1];			// how many valid bytes in packet
            data_fifo_bytes_read = data_fifo_bytes_read + length;
            
			if (length == 128)
			{
                // if data, put start- and end-of-packet headers
                data[1] = 0xAA; // start of packet
                data[130] = 0x55; // end of packet
                spi_write(data + 1, length + 2);
                //spi_write_with_NAK(data + 1, length+2);
                spi_bytes = spi_bytes + length;
			}
            else if (length == 4)
            {
                // if register, just send the 4 bytes
                //spi_write(data + 2, length);
                spi_write_with_NAK(data + 2, length);
                spi_bytes = spi_bytes + length;
            }
            else if (length==0xAA)
            {
                // for some reason, we received a packet with the SPI_DATA_TYPE still attached
            	aa_count++;
            }
            else if (length!=0)
            {
                // Some error occurred (perhaps CRC) and the length field is garbage
                bad_lengths++;
            }
			//finish_read_data();
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
			}
			clear_read_flag();
		}
	}
}