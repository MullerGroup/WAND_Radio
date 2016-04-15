/* spi_fifo.h
 *
 * CM-side fifo for commands to transmitted over SPI
 */

#ifndef SPI_FIFO_H
#define SPI_FIFO_H

#include <stdint.h>

#define SPI_FIFO_BYTES 135
#define SPI_FIFO_SIZE 10

#define COMMAND_VALID 0xAA

uint8_t *write_spi_fifo(void);
void finish_write_spi_fifo(void);
void reset_write_spi_fifo(void);
uint8_t *read_spi_fifo(void);
void finish_read_spi_fifo(void);
uint8_t get_capacity_spi_fifo(void);

#endif