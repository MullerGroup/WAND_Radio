/* command_fifo.c
 *
 * CM-side fifo for commands received from the GUI
 */

#include "command_fifo.h"
#include "radio_config.h"
#include <stdint.h>

uint8_t command_fifo[COMMAND_FIFO_SIZE][PACKET_SIZE];	// command fifo
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

void reset_write_command(void)
{
	if (c_write_ptr == 0)
	{
		c_write_ptr = COMMAND_FIFO_SIZE - 1;
	}
	else
	{
		c_write_ptr--;
	}
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

void reset_read_command(void)
{
	if (c_read_ptr == COMMAND_FIFO_SIZE - 1)
	{
		c_read_ptr = 0;
	}
	else
	{
		c_read_ptr++;
	}
}

// returns number of elements in fifo
uint8_t get_num_commands(void)
{
	return c_size;
}