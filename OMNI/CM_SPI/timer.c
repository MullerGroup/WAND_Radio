/* timer.c
 *
 * Timer used for timeout functions.
 */

#include "timer.h"
#include "radio_config.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "nrf.h"
#include "nrf_delay.h"
#include <stdbool.h>

bool error;
uint8_t empty_packet[PACKET_SIZE];


// Sets up the timer to interrupt on compare events
void init_timer(void)
{
	error = false;			// initialize with no error
	empty_packet[1] = 0;	// empty packet contains 0 bytes of data

	NRF_TIMER0->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;							// use timer mode
	NRF_TIMER0->BITMODE = TIMER_BITMODE_BITMODE_32Bit << TIMER_BITMODE_BITMODE_Pos;				// 32 bit counting
	NRF_TIMER0->PRESCALER = 4;																	// f = 16 MHz/2^4 = 1 MHz
	NRF_TIMER0->SHORTS = TIMER_SHORTS_COMPARE0_STOP_Enabled << TIMER_SHORTS_COMPARE0_STOP_Pos;	// stop the timer every time 
																								//	compare event happens
	NRF_TIMER0->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;		// enable compare interrupt
	
	NVIC_SetPriority(TIMER0_IRQn, 1); 
    NVIC_EnableIRQ(TIMER0_IRQn);
}

// Triggers an interrupt millis ms after this function call
void start_timeout(int millis)
{
	uint32_t compare;

	compare = millis*1000;
	NRF_TIMER0->CC[0] = compare;

	NRF_TIMER0->TASKS_CLEAR = 1;
	NRF_TIMER0->TASKS_START = 1;
}

void TIMER0_IRQHandler(void)
{
	uint8_t *receive_packet;
	uint8_t *packet_ptr;
	int timeout;
    bool packet_error = false;

	//uint32_t compare_0; 
	//uint32_t interrupts;
	//uint32_t mask;

	//compare_0 = NRF_TIMER0->EVENTS_COMPARE[0];
	//interrupts = NRF_TIMER0->INTENSET;
	//mask = TIMER_INTENSET_COMPARE0_Msk;

	if ((NRF_TIMER0->EVENTS_COMPARE[0] == 1) && (NRF_TIMER0->INTENSET & TIMER_INTENSET_COMPARE0_Msk))
	{
		NRF_TIMER0->EVENTS_COMPARE[0] = 0;	// clear the event

		// get location to put received commands
		receive_packet = write_command();
		// don't do anything if there is no room to write anyways
		if (receive_packet != 0)
		{
			radio_pause_tx();	// pause tx with new transmission
            NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
            
			
        	// wait for radio to be disabled
        	while ((NRF_RADIO->STATE & RADIO_STATE_STATE_Msk) != RADIO_STATE_STATE_Disabled)
        	{
        	}
            
            NRF_RADIO->INTENCLR = (RADIO_INTENCLR_END_Clear << RADIO_INTENCLR_END_Pos) |
                                    (RADIO_INTENCLR_READY_Clear << RADIO_INTENCLR_READY_Pos) |
                                    (RADIO_INTENCLR_DISABLED_Clear << RADIO_INTENCLR_DISABLED_Pos);

            NVIC_DisableIRQ(RADIO_IRQn);
	   
        	// now the radio is disabled, ready to go to Phase 2

        	// get a packet to send, or if there is no packet, send an empty packet
            if (radio_get_queued() == true)
            {
                packet_ptr = (uint8_t *)NRF_RADIO->PACKETPTR;
            }
            else
            {
        	    packet_ptr = read_data();
        	    if (packet_ptr == 0)
        	    {
        	        packet_ptr = empty_packet;
        	    }
                else
                {
                    radio_count();
                    
                    // adding for debugging
                    if (packet_ptr[1] == 128)
                    {
                        // if we got a valid data packet, check that its contents are correct
                        for (int j=2; j<66; j++)
                        {
                            packet_error = (packet_ptr[j]!=(get_prev_sample()+1));
                            if (packet_error)
                            {
                                // if there is an error, stop checking and clear variables
                                // also can set a breakpoint here to see what the incorrect packet contains
                                packet_error = false;
                                break;
                            }
                        }
                        set_prev_sample(packet_ptr[2]);
                    }

                    
                }
            }

        	// send either PHASE_2 or PHASE_2_ERROR signal
        	if (error == true)
        	{
        		packet_ptr[0] = PHASE_2_ERROR;
        	}
        	else
        	{
        		packet_ptr[0] = PHASE_2;
        	}

        	// transmit the packet
        	NRF_RADIO->PACKETPTR = (uint32_t)packet_ptr;
        	NRF_RADIO->EVENTS_DISABLED = 0;
        	NRF_RADIO->TASKS_TXEN = 1;
        	while (NRF_RADIO->EVENTS_DISABLED == 0)
        	{
        	}


        	// now we need to receive commands
        	NRF_RADIO->SHORTS = RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos;
        	NRF_RADIO->PACKETPTR = (uint32_t)receive_packet;
        	NRF_RADIO->EVENTS_END = 0;
        	timeout = PHASE_2_TIMEOUT;
        	NRF_RADIO->TASKS_RXEN = 1;
        	while(NRF_RADIO->EVENTS_END == 0)
        	{
        		if (timeout-- >= 0)
        		{
        			// continue counting down
        			nrf_delay_us(1000);
        		}
        		else
        		{
        			// done counting down -> timed out
        			break;
        		}
        	}

        	// check the outcome of receive with timeout

        	if (timeout >= 0)
        	{
        		// did not time out
        		if (NRF_RADIO->CRCSTATUS == 1)
        		{
        			// got CRC match, receive was good
        			error = false;
        			finish_write_command();
        		}
        		else
        		{
        			// no CRC match, don't want the received data, allow to overwrite
        			error = true;
                    reset_write_command();
        		}
        	}
        	else
        	{
        		// did timeout, need to re-request data next time
        		error = true;
                reset_write_command();
        	}

        	// disable the radio
        	NRF_RADIO->EVENTS_DISABLED = 0;
        	NRF_RADIO->TASKS_DISABLE = 1;
        	while(NRF_RADIO->EVENTS_DISABLED == 0)
        	{
        	}

            NRF_RADIO->EVENTS_END = 0;
            NRF_RADIO->EVENTS_READY = 0;

        	// done with receive, time to go back to transmitting
        	NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
        	                    (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);

        	NRF_RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos) |
                            	(RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos) |
                                (RADIO_INTENSET_DISABLED_Enabled << RADIO_INTENSET_DISABLED_Pos);

            radio_unpause_tx();
            NVIC_ClearPendingIRQ(RADIO_IRQn);
            NVIC_EnableIRQ(RADIO_IRQn);

            packet_ptr = read_data();
            if (packet_ptr != 0)
            {
                set_radio_disabled(false);
                NRF_RADIO->PACKETPTR = (uint32_t)packet_ptr;
                NRF_RADIO->TASKS_TXEN = 1;
                radio_count();
                
                // adding for debugging
                if (packet_ptr[1] == 128)
                {
                    // if we got a valid data packet, check that its contents are correct
                    for (int j=2; j<66; j++)
                    {
                        packet_error = (packet_ptr[j]!=(get_prev_sample()+1));
                        if (packet_error)
                        {
                            // if there is an error, stop checking and clear variables
                            // also can set a breakpoint here to see what the incorrect packet contains
                            packet_error = false;
                            break;
                        }
                    }
                    set_prev_sample(packet_ptr[2]);
                }
            }
        }
        else
        {
            if (((NRF_RADIO->STATE & RADIO_STATE_STATE_Msk) != RADIO_STATE_STATE_Disabled) && (get_radio_disabled() == true))
            {

                packet_ptr = read_data();
                if (packet_ptr != 0)
                {
                    set_radio_disabled(false);
                    NRF_RADIO->PACKETPTR = (uint32_t)packet_ptr;
                    NRF_RADIO->TASKS_TXEN = 1;
                    radio_count();
                    
                    // adding for debugging
                    if (packet_ptr[1] == 128)
                    {
                        // if we got a valid data packet, check that its contents are correct
                        for (int j=2; j<66; j++)
                        {
                            packet_error = (packet_ptr[j]!=(get_prev_sample()+1));
                            if (packet_error)
                            {
                                // if there is an error, stop checking and clear variables
                                // also can set a breakpoint here to see what the incorrect packet contains
                                packet_error = false;
                                break;
                            }
                        }
                        set_prev_sample(packet_ptr[2]);
                    }
                    
                }
            }
        }

        start_timeout(PHASE_1_LENGTH);
    }
}
