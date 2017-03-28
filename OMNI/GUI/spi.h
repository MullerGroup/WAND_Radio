/* spi.h
 * 
 * SPI interface with GUI.
 */

#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"

#define SPI_SCK_PIN			20
#define SPI_MOSI_PIN		21
#define SPI_MISO_PIN		22
#define SPI_CSN_PIN			23
#define SPI_RTR_PIN			24

#define DEF_CHARACTER		0xAA
#define ORC_CHARACTER		0x55

#define SPI_WRITE_LENGTH	(PACKET_SIZE)
#define CL_SPI_WRITE_LENGTH	(CL_PACKET_SIZE)
#define SPI_THRESHOLD		5


uint32_t spi_init(void);
void radio_spi_start(void);

#endif