/* spi_fifo.c
 *
 * CM-side fifo for commands to transmitted over SPI
 */

#include "spi_fifo.h"
#include <stdint.h>

uint8_t spi_fifo[SPI_FIFO_SIZE][SPI_FIFO_BYTES];
uint8_t s_read_ptr = 0;
uint8_t s_write_ptr = 0;
uint8_t s_size = 0;

uint8_t *write_spi_fifo(void)
{
	uint8_t *write_pointer;

	if (s_size < SPI_FIFO_SIZE)
	{
		// fifo is not full
		write_pointer = spi_fifo[s_write_ptr];
		s_write_ptr = (s_write_ptr + 1)%SPI_FIFO_SIZE;
		return write_pointer;
	}
	else
	{
		// fifo is full, return 0
		return 0;
	}
}

void finish_write_spi_fifo(void)
{
	// successfully written into fifo, increase size
	s_size++;
}

void reset_write_spi_fifo(void)
{
	if (s_write_ptr == 0)
	{
		s_write_ptr = SPI_FIFO_SIZE - 1;
	}
	else
	{
		s_write_ptr--;
	}
}

uint8_t *read_spi_fifo(void)
{
	uint8_t *read_pointer;

	if (s_size > 0)
	{
		// fifo is not empty
		read_pointer = spi_fifo[s_read_ptr];
		s_read_ptr = (s_read_ptr + 1)%SPI_FIFO_SIZE;
		return read_pointer;
	}
	else
	{
		// fifo is empty, return 0
		return 0;
	}
}

void finish_read_spi_fifo(void)
{
	// successfully read from fifo, decrease size
	s_size--;
}

uint8_t get_capacity_spi_fifo(void)
{
	return (SPI_FIFO_SIZE - s_size);
}
