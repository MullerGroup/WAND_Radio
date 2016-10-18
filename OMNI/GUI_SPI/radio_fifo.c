/* radio_fifo.c
 *
 * GUI-side fifo for data being sent from the CM to the GUI.
 */

#include "radio_fifo.h"
#include "radio_config.h"
#include <stdint.h>

uint8_t radio_fifo[RADIO_FIFO_SIZE][PACKET_SIZE+1];		// data fifo
uint8_t r_read_ptr = 0;								// head of fifo
uint8_t r_write_ptr = 0;								// tail of fifo
int 	r_size = 0;									// number of data elements in fifo

// gets pointer of element to write data into
uint8_t *write_radio(void)
{
	uint8_t *write_pointer;
	if (r_size < RADIO_FIFO_SIZE)
	{
		// fifo is not full
		write_pointer = radio_fifo[r_write_ptr];
		r_write_ptr = (r_write_ptr + 1)%RADIO_FIFO_SIZE;
		return write_pointer;
	}
	else
	{
		// fifo is full, return 0
		return 0;
	}
}

void finish_write_radio(void)
{
	r_size++;
}

// gets pointer of element to read data from
uint8_t *read_radio(void)
{
	uint8_t *read_pointer;
	if (r_size > 0)
	{
		// fifo is not empty
		read_pointer = radio_fifo[r_read_ptr];
		r_read_ptr = (r_read_ptr + 1)%RADIO_FIFO_SIZE;
		return read_pointer;
	}
	else
	{
		// fifo is empty, return 0
		return 0;
	}
}

void finish_read_radio(void)
{
	r_size--;
}
