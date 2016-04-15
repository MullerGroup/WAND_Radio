#ifndef RADIO_CONFIG_H
#define RADIO_CONFIG_H

#include <stdint.h>

#define PACKET_SIZE 100
#define NUM_PACKETS 1000
#define DEFAULT_FREQUENCY 50
#define TIMEOUT_LENGTH 50

void radio_configure(void);

uint32_t get_rssi(void);

void clear_rssi(void);

#endif