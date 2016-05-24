/* data_fifo.c
 *
 * GUI-side fifo for data being sent from the CM to the GUI.
 */

#include "data_fifo.h"
#include "radio_config.h"
#include "nrf.h"
#include <stdint.h>

uint8_t data_fifo[DATA_FIFO_SIZE][PACKET_SIZE];		// data fifo
uint8_t d_read_ptr = 0;								// head of fifo
uint8_t d_write_ptr = 0;							// tail of fifo
uint8_t d_size = 0;									// number of data elements in fifo
uint8_t d_full_count = 0;

// gets pointer of element to write data into
uint8_t *write_data(void)
{
	uint8_t *write_pointer;


		if (d_size < (DATA_FIFO_SIZE - 2))
		{
			// fifo is not full
			write_pointer = data_fifo[d_write_ptr];
			if ((uint32_t)write_pointer == NRF_RADIO->PACKETPTR)
			{
				d_full_count++;
				return 0;
			}
			else
			{
				d_write_ptr = (d_write_ptr + 1)%DATA_FIFO_SIZE;
				return write_pointer;
			}	
		}
		else
		{
			// fifo is full, return 0
			return 0;
		}
	
}

// updates pointers and size for writing into fifo
void finish_write_data(void)
{
	// successfully written into fifo, increase size
	d_size++;
}

void reset_write_data(void)
{
	if (d_write_ptr == 0)
	{
		d_write_ptr = DATA_FIFO_SIZE - 1;
	}
	else
	{
		d_write_ptr--;
	}
}

// gets pointer of element to read data from
uint8_t *read_data(void)
{
	uint8_t *read_pointer;

	if (d_size > 0)
	{
        // fifo is not empty
		read_pointer = data_fifo[d_read_ptr];
		d_read_ptr = (d_read_ptr + 1)%DATA_FIFO_SIZE;
		d_size--;
		return read_pointer;
	}
	else
	{
		// fifo is empty, return 0
		return 0;
	}
}


uint8_t get_num_data(void)
{
	return d_size;
}