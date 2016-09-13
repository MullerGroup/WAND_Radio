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

uint32_t debug_wd_count = 0;
uint32_t debug_fwd_count = 0;
uint32_t debug_rd_count = 0;
uint32_t debug_frd_count = 0;
int debug_size = 0;
uint32_t debug_full_count = 0;

// gets pointer of element to write data into
uint8_t *write_data(void)
{
	uint8_t *write_pointer;
	if (d_size < DATA_FIFO_SIZE-1)
	{
		debug_wd_count++;
		// fifo is not full
		write_pointer = data_fifo[d_write_ptr];
		d_write_ptr = (d_write_ptr + 1)%DATA_FIFO_SIZE;
		d_size++;
		return write_pointer;
	}
	else
	{
		debug_full_count++;
		// fifo is full, return 0
		return 0;
	}
}

// updates pointers and size for writing into fifo
void finish_write_data(void)
{
	// successfully written into fifo, increase size
	d_size++;
	debug_fwd_count++;
	debug_size++;
}

// gets pointer of element to read data from
uint8_t *read_data(void)
{
	uint8_t *read_pointer;

	if (d_size > 2)
	{
		debug_rd_count++;
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

// updates pointers and size for reading from fifo
void finish_read_data(void)
{
	// successfully read from fifo, decrease size
	d_size--;
	debug_frd_count++;
	debug_size--;
}

// updates pointers and size for reading from fifo
uint8_t get_num_data(void)
{
	return d_size;
}