/* radio_config.c
 * 
 * CM-side radio
 */

#include "radio_config.h"
#include "nrf_delay.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include <stdbool.h>
#include <stdint.h>

#define PACKET0_S1_SIZE             (0UL)  //!< S1 size in bits
#define PACKET0_S0_SIZE             (0UL)  //!< S0 size in bits
#define PACKET0_PAYLOAD_SIZE        (0UL)  //!< payload size in bits
#define PACKET1_BASE_ADDRESS_LENGTH (4UL)  //!< base address length in bytes
#define PACKET1_STATIC_LENGTH       (PACKET_SIZE)  //!< static length in bytes
#define PACKET1_PAYLOAD_SIZE        (PACKET_SIZE)  //!< payload size in bytes

bool timer_irq;
bool packet_queued;
bool radio_disabled;
uint32_t radio_bytes;
uint32_t aa_count_radio;

// // for debugging, checking if packet is correct
// uint8_t prev_sample = 0;
// bool packet_error = false;

void aa()
{
    aa_count_radio++;
}

void radio_configure()
{
    radio_bytes = 0;
    packet_queued = false;
    radio_disabled = true;

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
                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);

    NRF_RADIO->INTENSET = (RADIO_INTENSET_END_Enabled << RADIO_INTENSET_END_Pos) |
                            (RADIO_INTENSET_READY_Enabled << RADIO_INTENSET_READY_Pos) |
                            (RADIO_INTENSET_DISABLED_Enabled << RADIO_INTENSET_DISABLED_Pos);

    NVIC_SetPriority(RADIO_IRQn, 0); //Highest priority
    NVIC_EnableIRQ(RADIO_IRQn);

    nrf_delay_ms(3);
}

// //for debug
// uint8_t get_prev_sample(void)
// {
//     return prev_sample;
// }

// void set_prev_sample(uint8_t sample)
// {
//     prev_sample = sample;
// }

void radio_pause_tx(void)
{
    timer_irq = true;
}

void radio_unpause_tx(void)
{
    timer_irq = false;
}

bool radio_get_queued(void)
{
    return packet_queued;
}

void set_radio_disabled(bool status)
{
    radio_disabled = status;
}

bool get_radio_disabled(void)
{
    return radio_disabled;
}

void RADIO_IRQHandler(void)
{ 
    uint8_t *newPacketPtr;

    if((NRF_RADIO->EVENTS_READY == 1) && (NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk))
    {
        NRF_RADIO->EVENTS_READY = 0;    // clear the event

        if (timer_irq != true)
        {
            // not currently in a phase switch, so behave normally
            newPacketPtr = read_data();
            if (newPacketPtr != 0)
            {
                if (newPacketPtr[1]==0xAA)
                {
                    aa();
                }
                // have another packet to transmit
                NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                    (RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos);
                NRF_RADIO->PACKETPTR = (uint32_t)newPacketPtr;
                packet_queued = true;
                radio_count();
                
                // // adding for debugging
                // if (newPacketPtr[1] == 128)
                // {
                //     // if we got a valid data packet, check that its contents are correct
                //     for (int j=2; j<66; j++)
                //     {
                //         packet_error = (newPacketPtr[j]!=(prev_sample+1));
                //         if (packet_error)
                //         {
                //             // if there is an error, stop checking and clear variables
                //             // also can set a breakpoint here to see what the incorrect packet contains
                //             packet_error = false;
                //             break;
                //         }
                //     }
                //     prev_sample = newPacketPtr[2];
                // }

            }
            else
            {
                // no more packets
                NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                    (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
                packet_queued = false;
            }
        }
    }

    if((NRF_RADIO->EVENTS_END == 1) && (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk))
    {
        NRF_RADIO->EVENTS_END = 0;  // clear the event

        if (timer_irq != true)
        {
            if (packet_queued)
            {
                // not currently in a phase switch, so behave normally
                newPacketPtr = read_data();
                if (newPacketPtr != 0)
                {
                    if (newPacketPtr[1]==0xAA)
                    {
                        aa();
                    }
                    // have another packet to transmit
                    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                        (RADIO_SHORTS_END_START_Enabled << RADIO_SHORTS_END_START_Pos);
                    NRF_RADIO->PACKETPTR = (uint32_t)newPacketPtr;
                    packet_queued = true;
                    radio_count();
                    
                    // // adding for debugging
                    // if (newPacketPtr[1] == 128)
                    // {
                    //     // if we got a valid data packet, check that its contents are correct
                    //     for (int j=2; j<66; j++)
                    //     {
                    //         packet_error = (newPacketPtr[j]!=(prev_sample+1));
                    //         if (packet_error)
                    //         {
                    //             // if there is an error, stop checking and clear variables
                    //             // also can set a breakpoint here to see what the incorrect packet contains
                    //             packet_error = false;
                    //             break;
                    //         }
                    //     }
                    //     prev_sample = newPacketPtr[2];
                    // }
                    
                }
                else
                {
                    // no more packets
                    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) | 
                                        (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);
                    packet_queued = false;
                }
            }
        }
    }

    if((NRF_RADIO->EVENTS_DISABLED == 1) && (NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk))
    {
        NRF_RADIO->EVENTS_DISABLED = 0;

        if (timer_irq != true)
        {
            set_radio_disabled(true);
            packet_queued = false;
        }
    }


}

void radio_count(void)
{
    radio_bytes = radio_bytes + 128;
}




