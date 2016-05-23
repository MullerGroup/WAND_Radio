/* radio_config.c
 * 
 * GUI-side radio
 */

 /* Bugs:

 2/28/16:

 Experiencing a strange bug on turnarounds. Whenever the radio is disabled, 
 turned around to Tx, then re-enabled to Rx, there will be an END interrupt
 generated even when there is no end event.



 */

#include "radio_config.h"
#include "nrf_delay.h"
#include "data_fifo.h"
#include "command_fifo.h"
#include "spi.h"
#include <stdbool.h>

#define PACKET0_S1_SIZE             (0UL)  //!< S1 size in bits
#define PACKET0_S0_SIZE             (0UL)  //!< S0 size in bits
#define PACKET0_PAYLOAD_SIZE        (0UL)  //!< payload size in bits
#define PACKET1_BASE_ADDRESS_LENGTH (4UL)  //!< base address length in bytes
#define PACKET1_STATIC_LENGTH       (PACKET_SIZE)  //!< static length in bytes
#define PACKET1_PAYLOAD_SIZE        (PACKET_SIZE)  //!< payload size in bytes

uint8_t *newPacketPtr;
uint8_t command_packet[PACKET_SIZE];
uint8_t overflow_packet[PACKET_SIZE];
bool 	overflow1 = false;
bool    overflow2 = false;
uint8_t *rec_packet1;
uint8_t *rec_packet2;

uint32_t radio_bytes;
uint32_t radio_bytes_total;

int crc_count = 0;

// for debugging, checking if packet is correct
uint8_t prev_sample = 0;
bool packet_error = false;
uint32_t packets_received = 0;

void radio_configure()
{
    radio_bytes = 0;
    radio_bytes_total = 0;

    // Radio config
    NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_Pos4dBm << RADIO_TXPOWER_TXPOWER_Pos);
    //NRF_RADIO->TXPOWER = (RADIO_TXPOWER_TXPOWER_0dBm << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->FREQUENCY = 7UL;  // Frequency bin 7, 2407MHz
    NRF_RADIO->MODE = (RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos);

    // Radio address config
    NRF_RADIO->PREFIX0     = 0xC4C3C2E7UL;  // Prefix byte of addresses 3 to 0
    NRF_RADIO->PREFIX1     = 0xC5C6C7C8UL;  // Prefix byte of addresses 7 to 4
    NRF_RADIO->BASE0       = 0xE7E7E7E7UL;  // Base address for prefix 0
    NRF_RADIO->BASE1       = 0x00C2C2C2UL;  // Base address for prefix 1-7
    NRF_RADIO->TXADDRESS   = 0x00UL;      // Set device address 0 to use when transmitting
    NRF_RADIO->RXADDRESSES = 0x01UL;    // Enable device address 0 to use which receiving

    // Packet configuration
    NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE << RADIO_PCNF0_S1LEN_Pos) |
                       (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) |
                       (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

    // Packet configuration
    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)        |
                       (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)    |
                       (PACKET1_STATIC_LENGTH << RADIO_PCNF1_STATLEN_Pos)        |
                       (PACKET1_PAYLOAD_SIZE << RADIO_PCNF1_MAXLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

    // CRC Config
    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
    if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFFFUL;      // Initial value      
        NRF_RADIO->CRCPOLY = 0x11021UL;     // CRC poly: x^16+x^12^x^5+1
    }
    else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFUL;        // Initial value
        NRF_RADIO->CRCPOLY = 0x107UL;       // CRC poly: x^8+x^2^x^1+1
    }

    // Initial Shortcuts: default to receive data, so ready-start and end-start

    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                        (RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos);

    NRF_RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos) |
                            (RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos);

    NVIC_SetPriority(RADIO_IRQn, 0); //Highest priority
    NVIC_EnableIRQ(RADIO_IRQn);

    newPacketPtr = write_data();		              // get first element of packet fifo
    if (newPacketPtr != 0)
    {
        NRF_RADIO->PACKETPTR = (uint32_t)newPacketPtr;	 // write its address as radio packet pointer
        rec_packet1 = newPacketPtr;
        overflow1 = false;
    }
    else 
    {
        NRF_RADIO->PACKETPTR = (uint32_t)overflow_packet;
        rec_packet1 = overflow_packet;
        overflow1 = true;
    }

    nrf_delay_ms(3);

    NRF_RADIO->TASKS_RXEN = 1U; // enable the radio to start listening
}

void RADIO_IRQHandler(void)
{ 
	uint8_t *command;
	uint8_t i;
	uint8_t command_index;
	uint8_t command_count;
    


    if ((NRF_RADIO->EVENTS_READY == 1) && (NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk))
    {


        // clear the event
        

        // check if we have more data to send
        newPacketPtr = write_data();
        if (newPacketPtr == 0)
        {
            // buffer was full, use overflow packet for now
            NRF_RADIO->PACKETPTR = (uint32_t)overflow_packet;
            rec_packet2 = overflow_packet;
            overflow2 = true;
        }
        else
        {
            // got a valid packet pointer
            NRF_RADIO->PACKETPTR = (uint32_t)newPacketPtr;
            rec_packet2 = newPacketPtr;
            overflow2 = false;
        }
        //nrf_delay_ms(1);
        NRF_RADIO->EVENTS_READY = 0;
    }


	if ((NRF_RADIO->EVENTS_END == 1) && (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk))
    {

        if(NRF_RADIO->CRCSTATUS != 1)
        {
            crc_count++;
        }

    	// finish fifo write if necessary
    	if (overflow1 != true)
    	{
    		finish_write_data();
            radio_bytes = radio_bytes + rec_packet1[1];
    	}
        
        // // adding for debugging
        // if (rec_packet1[1] == 128)
        // {
        //     // if we got a valid data packet, check that its contents are correct
        //     for (int j=2; j<66; j++)
        //     {
        //         packet_error = (rec_packet1[j]!=(prev_sample+1)%256);
        //         if (packet_error)
        //         {
        //             // if there is an error, stop checking and clear variables
        //             // also can set a breakpoint here to see what the incorrect packet contains
        //             packet_error = false;
        //             break;
        //         }
        //     }
        //     prev_sample = rec_packet1[2];
        // }

        radio_bytes_total = radio_bytes_total + rec_packet1[1];
        packets_received = packets_received + 1;

        overflow1 = overflow2;

    	// check what type of packet it is
    	if ((rec_packet1[0] == PHASE_2) || (rec_packet1[0] == PHASE_2_ERROR))
    	{
            NVIC_DisableIRQ(RADIO_IRQn);
            set_read_flag();



    		// Packet is some sort of PHASE 2 packet requesting commands

            // set shorts for tx and get rid of interrupts
            NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
            NRF_RADIO->INTENCLR = (RADIO_INTENCLR_END_Clear << RADIO_INTENCLR_END_Pos) |
                                    (RADIO_INTENCLR_READY_Clear << RADIO_INTENCLR_READY_Pos);

    		// disable the radio since we need to turn around for tx
    		NRF_RADIO->EVENTS_DISABLED = 0U;
            NRF_RADIO->TASKS_DISABLE = 1;
            while (NRF_RADIO->EVENTS_DISABLED == 0U)
            {
            }


    		if (rec_packet1[0] == PHASE_2)
    		{
    			// CM received last set of commands, so clear to send new set now
    			// clear command_packet buffer
    			for (i=0;i<PACKET_SIZE;i++)
    			{
    				command_packet[i] = 0;
    			}
    			// fill command_packet buffer with commands from command fifo
    			command_index = 1; // use 0th byte as number of commands
    			command_count = 0;
                while (command_count < MAX_COMMANDS)
                {
                    command = read_command();
                    if (command == 0)
                    {
                        break;
                    }
                    else
                    {
                        for (i=0;i<COMMAND_SIZE;i++)
                        {
                            command_packet[command_index + i] = command[i];
                        }
                        command_index = command_index + COMMAND_SIZE;
                        finish_read_command();
                    }
                    command_count++;
                }
    			command_packet[0] = command_count*COMMAND_SIZE;
    		}
    		// if PHASE_2_ERROR, don't need to do anything, just resend the current command_packet

    		NRF_RADIO->PACKETPTR = (uint32_t)command_packet;
            

    		// transmit the command packet
    		NRF_RADIO->EVENTS_DISABLED = 0U;
            NRF_RADIO->TASKS_TXEN = 1;
            while (NRF_RADIO->EVENTS_DISABLED == 0U)
            {
            }
            NRF_RADIO->EVENTS_END = 0;
            NRF_RADIO->EVENTS_READY = 0;

            // reset the shorts and interrupts we want for Rx
            NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                        		(RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos);

    		NRF_RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos) |
                                    (RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos);

            NRF_RADIO->PACKETPTR = (uint32_t)rec_packet2;
            rec_packet1 = rec_packet2;
            NVIC_ClearPendingIRQ(RADIO_IRQn);
            NVIC_EnableIRQ(RADIO_IRQn);

    		NRF_RADIO->TASKS_RXEN = 1;


    	}
    	else
    	{
    		// Phase 1: CM will continue sending packets, so try to get a new packet pointer and continue in Rx

            rec_packet1 = rec_packet2;

    		newPacketPtr = write_data();
    		if (newPacketPtr == 0)
    		{
    			// buffer was full, use overflow packet for now
    			NRF_RADIO->PACKETPTR = (uint32_t)overflow_packet;
    			rec_packet2 = overflow_packet;
    			overflow2 = true;
    		}
    		else
    		{
    			// got a valid packet pointer
    			NRF_RADIO->PACKETPTR = (uint32_t)newPacketPtr;
    			rec_packet2 = newPacketPtr;
    			overflow2 = false;
    		}
    	}

        // clear the event
        NRF_RADIO->EVENTS_END = 0;
    }
}




