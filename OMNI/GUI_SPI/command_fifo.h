/* command_fifo.h
 *
 * GUI-side fifo for commands being sent from the GUI to the CM.
 */

#ifndef COMMAND_FIFO_H
#define COMMAND_FIFO_H

#include <stdint.h>

#define COMMAND_SIZE 5				// size of each command in bytes
#define COMMAND_FIFO_SIZE 50		// size of the fifo in commands
#define MAX_COMMANDS 2				// PACKET_SIZE/5

uint8_t *write_command(void);		// used to write command into the fifo
void finish_write_command(void);

uint8_t *read_command(void);		// used to read command from the fifo

#endif