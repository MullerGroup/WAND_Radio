/* data_fifo.h
 *
 * GUI-side fifo for data being sent from the CM to the GUI.
 */

#ifndef DATA_FIFO_H
#define DATA_FIFO_H

#include <stdint.h>

#define DATA_FIFO_SIZE 46			// size of fifo in data packets

uint8_t *write_data(void);			// used to write data into the fifo
void finish_write_data(void);
void reset_write_data(void);

uint8_t *read_data(void);			// used to read data from the fifo

uint8_t get_num_data(void);			// used to read how many data elements are in fifo 

#endif