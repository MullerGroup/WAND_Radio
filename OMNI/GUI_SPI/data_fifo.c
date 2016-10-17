/* data_fifo.c
 *
 * GUI-side fifo for data being sent from the CM to the GUI.
 */

#include "data_fifo.h"
#include "radio_config.h"
#include <stdint.h>

uint8_t data_fifo[DATA_FIFO_SIZE][PACKET_SIZE+1];		// data fifo
uint8_t d_read_ptr = 0;								// head of fifo
uint8_t d_write_ptr = 0;								// tail of fifo
int 	d_size = 0;									// number of data elements in fifo

// gets pointer of element to write data into
uint8_t *write_data(void)
{
	uint8_t *write_pointer;
	if (d_size < DATA_FIFO_SIZE-1)
	{
		// fifo is not full
		write_pointer = data_fifo[d_write_ptr];
		d_write_ptr = (d_write_ptr + 1)%DATA_FIFO_SIZE;
		d_size++;
		return write_pointer;
	}
	else
	{
		// fifo is full, return 0
		return 0;
	}
}

// gets pointer of element to read data from
uint8_t *read_data(void)
{
	uint8_t dlen;
	uint8_t *read_pointer;
	dlen = 0;
	while (dlen == 0)
	{
		if (d_size > 2)
		{
			// fifo is not empty
			read_pointer = data_fifo[d_read_ptr];
			d_read_ptr = (d_read_ptr + 1)%DATA_FIFO_SIZE;
			d_size--;
			dlen = read_pointer[1];
		}
		else
		{
			// fifo is empty, return 0
			return 0;
		}
	}
	return read_pointer;
}

void unread_data(void)
{
	d_size++;
	if (d_read_ptr == 0)
	{
		d_read_ptr = DATA_FIFO_SIZE-1;
	}
	else
	{
		d_read_ptr--;
	}
}

// updates pointers and size for reading from fifo
uint8_t get_num_data(void)
{
	return d_size;
}