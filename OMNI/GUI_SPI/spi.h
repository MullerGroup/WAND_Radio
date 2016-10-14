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
#define SPI_CSN_PIN			1
#define SPI_RTR_PIN			4

#define DEF_CHARACTER		0x00
#define ORC_CHARACTER		0x00

#define SPI_WRITE_LENGTH	198
#define SPI_THRESHOLD		5


uint32_t spi_init(void);
void radio_spi_start(void);

#endif