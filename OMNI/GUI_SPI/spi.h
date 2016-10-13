/* spi.h
 * 
 * SPI interface with GUI.
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>

#define SPI_SCK_PIN			0
#define SPI_MOSI_PIN		2
#define SPI_MISO_PIN		3
#define SPI_SS_PIN			1
#define SPI_RTR_PIN			4


void set_read_flag(void);
void clear_read_flag(void);
bool get_read_flag(void);
void spi_write(uint8_t buffer[], uint8_t size);
void spi_read(uint8_t buffer[], uint8_t size);
bool spi_read_with_NAK(uint8_t buffer[], uint8_t size);
void spi_write_with_NAK(uint8_t buffer[], uint8_t size);
void spi_init(void);

#endif