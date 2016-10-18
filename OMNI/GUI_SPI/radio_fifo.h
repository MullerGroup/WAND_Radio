/* data_fifo.h
 *
 * GUI-side fifo for data being sent from the CM to the GUI.
 */

#ifndef RADIO_FIFO_H
#define RADIO_FIFO_H

#include <stdint.h>

#define RADIO_FIFO_SIZE 5			// size of fifo in data packets

uint8_t *write_radio(void);			// used to write data into the fifo
void finish_write_radio(void);

uint8_t *read_radio(void);			// used to read data from the fifo
void finish_read_radio(void);

#endif