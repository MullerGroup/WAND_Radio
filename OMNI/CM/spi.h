#ifndef SPI_H
#define SPI_H

#include <stdint.h>
#include "spi_slave.h"
#include "radio_config.h"

#define SPI_DATA		0xAA
#define SPI_REGISTER	0xFF
#define SPI_DATA_LENGTH (PACKET_SIZE - 2)
#define CL_SPI_DATA_LENGTH (CL_PACKET_SIZE - 2)
#define SPI_REGISTER_LENGTH 4

#define SPI_PACKET_BYTES (SPI_DATA_LENGTH + 7)

#define DEF_CHARACTER 0x00u
#define ORC_CHARACTER 0x00u

#define RADIO_THRESHOLD 0

/*
Radio Threshold Values Notes

0:	CM Nordic "thinks" that it's transmitted all packets, but GUI Nordic is only 
		receiving exactly half of them
	Possible that CM Nordic is only actually transmitting every other packet and
		the radio is somehow getting screwed up by the SPI

*/


void spi_slave_event_handle(spi_slave_evt_t event);
uint32_t spi_init(void);

#endif 