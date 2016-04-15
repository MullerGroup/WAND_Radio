/* radio_config.h
 * 
 * GUI-side radio
 */

 #ifndef RADIO_CONFIG_H
 #define RADIO_CONFIG_H

 #define PACKET_SIZE 130

 #define PHASE_1 		0
 #define PHASE_2 		1
 #define PHASE_2_ERROR 	2

 void radio_configure(void);

 #endif