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
    
//    bool packet_error = false;
//    uint8_t prev_sample = 0;
    uint8_t debug_packet[PACKET_SIZE];
    debug_packet[0] = 0;
    debug_packet[1] = 128;
    for (int i=2; i<66; i++)
    {
        debug_packet[i] = 0;
    }
    
    
	while (true)
	{
		// check data fifo for data to transmit over UART
		
		
			// there was data to be sent over UART
//        data = read_data();
        
        // added for debugging dropped bytes
        nrf_delay_ms(1);
        for (int i=2; i<66; i++)
        {
            debug_packet[i]+=1; // increment by 1
        }
        data = debug_packet;
        
        if (data != 0)
        {
			length = data[1];			// how many valid bytes in packet
            
            // adding for debugging
//            if (length == 128)
//            {
//                // if we got a valid data packet, check that its contents are correct
//                for (int j=2; j<66; j++)
//                {
//                    packet_error = (data[j]!=(prev_sample+1));
//                    if (packet_error)
//                    {
//                        // if there is an error, stop checking and clear variables
//                        // also can set a breakpoint here to see what the incorrect packet contains
//                        packet_error = false;
//                        break;
//                    }
//                }
//                prev_sample = data[2];
//            }
            
			if (length > 0)
			{
				spi_write_with_NAK(data + 2, length);
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