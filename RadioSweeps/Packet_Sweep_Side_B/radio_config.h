#ifndef RADIO_CONFIG_H
#define RADIO_CONFIG_H

#include <stdint.h>

#define DEFAULT_PACKET_SIZE 30
#define NUM_PACKETS 1000
#define DEFAULT_FREQUENCY 0
#define TIMEOUT_LENGTH 50
#define MAX_PACKET_SIZE 200

void radio_configure(uint8_t size);


#endif