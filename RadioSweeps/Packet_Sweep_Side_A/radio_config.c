#include "radio_config.h"
#include "nrf_delay.h"
#include <stdint.h>

#define PACKET0_S1_SIZE             (0UL)  //!< S1 size in bits
#define PACKET0_S0_SIZE             (0UL)  //!< S0 size in bits
#define PACKET0_PAYLOAD_SIZE        (0UL)  //!< payload size in bits
#define PACKET1_BASE_ADDRESS_LENGTH (4UL)  //!< base address length in bytes
#define PACKET1_STATIC_LENGTH       (DEFAULT_PACKET_SIZE)  //!< static length in bytes
#define PACKET1_PAYLOAD_SIZE        (DEFAULT_PACKET_SIZE)  //!< payload size in bytes

void radio_configure(uint8_t size)
{
    // Radio config
    NRF_RADIO->TXPOWER   = (RADIO_TXPOWER_TXPOWER_Neg4dBm << RADIO_TXPOWER_TXPOWER_Pos);
    NRF_RADIO->FREQUENCY = DEFAULT_FREQUENCY;                // Frequency bin 7, 2407MHz
    NRF_RADIO->MODE      = (RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos);

    // Radio address config
    NRF_RADIO->PREFIX0     = 0xC4C3C2E7UL;  // Prefix byte of addresses 3 to 0
    NRF_RADIO->PREFIX1     = 0xC5C6C7C8UL;  // Prefix byte of addresses 7 to 4
    NRF_RADIO->BASE0       = 0xE7E7E7E7UL;  // Base address for prefix 0
    NRF_RADIO->BASE1       = 0x00C2C2C2UL;  // Base address for prefix 1-7
    NRF_RADIO->TXADDRESS   = 0x00UL;        // Set device address 0 to use when transmitting
    NRF_RADIO->RXADDRESSES = 0x01UL;        // Enable device address 0 to use which receiving

    // Packet configuration
    NRF_RADIO->PCNF0 = (PACKET0_S1_SIZE << RADIO_PCNF0_S1LEN_Pos) |
                       (PACKET0_S0_SIZE << RADIO_PCNF0_S0LEN_Pos) |
                       (PACKET0_PAYLOAD_SIZE << RADIO_PCNF0_LFLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

    // Packet configuration
    NRF_RADIO->PCNF1 = (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos) |
                       (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)        |
                       (PACKET1_BASE_ADDRESS_LENGTH << RADIO_PCNF1_BALEN_Pos)    |
                       (size << RADIO_PCNF1_STATLEN_Pos)        |
                       (size << RADIO_PCNF1_MAXLEN_Pos); //lint !e845 "The right argument to operator '|' is certain to be 0"

    // CRC Config
    NRF_RADIO->CRCCNF = (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos); // Number of checksum bits
    if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_Two << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFFFUL;   // Initial value      
        NRF_RADIO->CRCPOLY = 0x11021UL;  // CRC poly: x^16+x^12^x^5+1
    }
    else if ((NRF_RADIO->CRCCNF & RADIO_CRCCNF_LEN_Msk) == (RADIO_CRCCNF_LEN_One << RADIO_CRCCNF_LEN_Pos))
    {
        NRF_RADIO->CRCINIT = 0xFFUL;   // Initial value
        NRF_RADIO->CRCPOLY = 0x107UL;  // CRC poly: x^8+x^2^x^1+1
    }

    NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos);

    nrf_delay_ms(3);
}

