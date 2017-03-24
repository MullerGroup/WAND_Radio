/* radio_config.h
 * 
 * CM-side radio
 */

 #include <stdbool.h>
#include <stdint.h>

 #ifndef RADIO_CONFIG_H
 #define RADIO_CONFIG_H

 #define PACKET_SIZE 	200

 #define PHASE_1 		0
 #define PHASE_2 		1
 #define PHASE_2_ERROR 	2

 #define PHASE_2_TIMEOUT 50

 void radio_configure(void);

 void radio_pause_tx(void);

 void radio_unpause_tx(void);

 bool radio_get_queued(void);

 void set_radio_disabled(bool status);

 bool get_radio_disabled(void);

 #endif