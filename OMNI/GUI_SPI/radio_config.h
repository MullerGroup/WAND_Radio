/* radio_config.h
 * 
 * GUI-side radio
 */

 #ifndef RADIO_CONFIG_H
 #define RADIO_CONFIG_H

 #define PACKET_SIZE 130

 #define PHASE_1 		0 // keep listening for data
 #define PHASE_2 		1 // turn around and send new packets (commands)
 #define PHASE_2_ERROR 	2 // turn around and resend old packets (commands)

 void radio_configure(void);

 #endif