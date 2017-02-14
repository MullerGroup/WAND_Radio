/* command_fifo.h
 *
 * CM-side fifo for commands received from the GUI
 */

#ifndef COMMAND_FIFO_H
#define COMMAND_FIFO_H

#include <stdint.h>
#include "radio_config.h"

#define COMMAND_SIZE 5				// size of each command in bytes
#define COMMAND_FIFO_SIZE 15		// size of the fifo in commands
#define MAX_COMMANDS 2				// PACKET_SIZE/5

uint8_t *write_command(void);		// used to write command into the fifo
void finish_write_command(void);
void reset_write_command(void);

uint8_t *read_command(void);		// used to read command from the fifo
void finish_read_command(void);
void reset_read_command(void);

uint8_t get_num_commands(void);		// used to read how many commands are in fifo

#endif