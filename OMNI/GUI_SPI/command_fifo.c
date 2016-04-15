/* command_fifo.c
 *
 * GUI-side fifo for commands being sent from the GUI to the CM.
 */

#include "command_fifo.h"
#include <stdint.h>

uint8_t command_fifo[COMMAND_FIFO_SIZE][COMMAND_SIZE];	// command fifo
uint8_t c_read_ptr = 0;									// head of fifo
uint8_t c_write_ptr = 0;									// tail of fifo
uint8_t c_size = 0;										// number of commands in fifo

// gets pointer of element to write command into
uint8_t *write_command(void)
{
	uint8_t *write_pointer;

	if (c_size < COMMAND_FIFO_SIZE)
	{
		// fifo is not full
		write_pointer = command_fifo[c_write_ptr];
		c_write_ptr = (c_write_ptr + 1)%COMMAND_FIFO_SIZE;
		return write_pointer;
	}
	else
	{
		// fifo is full, return 0
		return 0;
	}
}

// updates pointers and size for writing into fifo
void finish_write_command(void)
{
	// successfully written into fifo, increase size 
	c_size++;
}

// gets pointer of element to read command from
uint8_t *read_command(void)
{
	uint8_t *read_pointer;

	if (c_size > 0)
	{
		// fifo is not empty
		read_pointer = command_fifo[c_read_ptr];
		c_read_ptr = (c_read_ptr + 1)%COMMAND_FIFO_SIZE;
		return read_pointer;
	}
	else
	{
		// fifo is empty, return 0
		return 0;
	}
}

// updates pointers and size for reading from fifo
void finish_read_command(void)
{
	// successfully read from fifo, decrease size
	c_size--;
}

// returns number of elements in fifo
uint8_t get_num_commands(void)
{
	return c_size;
}