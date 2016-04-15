#include "uart.h"
#include "radio_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

uint8_t f_packet[MAX_PACKET_SIZE];
uint8_t tx_packet[MAX_PACKET_SIZE];
uint8_t rx_packet[MAX_PACKET_SIZE];


void simple_uart_put(uint8_t cr)
{
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}

void simple_uart_putbuf(uint8_t buffer[], uint8_t size)
{
	uint8_t i;

	for (i=0;i<size;i++)
	{
		simple_uart_put(buffer[i]);
	}
}

void simple_uart_putstring(const uint8_t * str)
{
    uint_fast8_t i  = 0;
    uint8_t      ch = str[i++];

    while (ch != '\0')
    {
        simple_uart_put(ch);
        ch = str[i++];
    }
}

void simple_uart_config(uint8_t rts_pin_number,
                        uint8_t txd_pin_number,
                        uint8_t cts_pin_number,
                        uint8_t rxd_pin_number,
                        bool    hwfc)
{
	int i;
	for (i=0;i<MAX_PACKET_SIZE;i++)
	{
		tx_packet[i] = i;
	}

/** @snippet [Configure UART RX and TX pin] */
    nrf_gpio_cfg_output(txd_pin_number);
    nrf_gpio_cfg_input(rxd_pin_number, NRF_GPIO_PIN_NOPULL);

    NRF_UART0->PSELTXD = txd_pin_number;
    NRF_UART0->PSELRXD = rxd_pin_number;
/** @snippet [Configure UART RX and TX pin] */
    if (hwfc)
    {
        nrf_gpio_cfg_output(rts_pin_number);
        nrf_gpio_cfg_input(cts_pin_number, NRF_GPIO_PIN_NOPULL);
        NRF_UART0->PSELCTS = cts_pin_number;
        NRF_UART0->PSELRTS = rts_pin_number;
        NRF_UART0->CONFIG  = (UART_CONFIG_HWFC_Enabled << UART_CONFIG_HWFC_Pos);
    }

    NRF_UART0->BAUDRATE      = (UART_BAUDRATE_BAUDRATE_Baud38400 << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);

    NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos);

    NVIC_SetPriority(UART0_IRQn, 0); //Highest priority
    NVIC_ClearPendingIRQ(UART0_IRQn);
    NVIC_EnableIRQ(UART0_IRQn);

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;

    nrf_delay_ms(3);
}

void UART0_IRQHandler(void)
{
  uint8_t data;
	bool success;
	int x;
	uint8_t size;
	int i;
	bool crc;

	if ((NRF_UART0->EVENTS_RXDRDY == 1) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
	{
		NRF_UART0->EVENTS_RXDRDY = 0;
    data = NRF_UART0->RXD;
    if (data == '0')
    {
      simple_uart_putstring((const uint8_t *)"Packet Size, Dropped Packets, Good Packets \n\r");
     for (size=DEFAULT_PACKET_SIZE;size<(MAX_PACKET_SIZE+1);size=size+10)
     { 
      

			// STAGE 1: Transmit the desired frequency
			f_packet[0] = size;

			NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
        						      (RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos);

       		success = false;
        	while (success == false)
        	{
        		// transmit
        		NRF_RADIO->PACKETPTR = (uint32_t)f_packet;
        		NRF_RADIO->EVENTS_DISABLED = 0;
        		NRF_RADIO->TASKS_TXEN = 1;
        		while (NRF_RADIO->EVENTS_DISABLED == 0)
        		{
        		}
        		// now wait for an ack packet
        		x = 1000;
        		NRF_RADIO->PACKETPTR = (uint32_t)rx_packet;
        		NRF_RADIO->EVENTS_DISABLED = 0;
        		NRF_RADIO->TASKS_RXEN = 1;
       			while (NRF_RADIO->EVENTS_DISABLED == 0)
       			{
       				if (x-- >= 0)
       				{
       					nrf_delay_us(100);
       				}
       				else
       				{
       					break;
       				}
       			}

       			if (x >= 0)
       			{
       				success = true;
       			}
       			else
       			{
       				NRF_RADIO->EVENTS_DISABLED = 0;
       				NRF_RADIO->TASKS_DISABLE = 1;
       				while (NRF_RADIO->EVENTS_DISABLED == 0)
       				{
       				}
       			}
       		}

       		// change the frequency
       		radio_configure(size);

       		// STAGE 2: Send 1000 packets
       		nrf_delay_ms(1);

       		NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos);
       		NRF_RADIO->PACKETPTR = (uint32_t)tx_packet;
       		NRF_RADIO->EVENTS_END = 0;
       		NRF_RADIO->TASKS_TXEN = 1;

       		for (i=0;i<NUM_PACKETS;i++)
       		{
       			while (NRF_RADIO->EVENTS_END == 0)
       			{
       			}
       			nrf_delay_us(100);
       			NRF_RADIO->EVENTS_END = 0;
       			NRF_RADIO->TASKS_START = 1;
       		}
          // disable the radio
          NRF_RADIO->EVENTS_DISABLED = 0;
          NRF_RADIO->TASKS_DISABLE = 1;
          while (NRF_RADIO->EVENTS_DISABLED == 0)
          {
          }

       		// STAGE 3: Listen for results and output them on UART

       		NRF_RADIO->SHORTS = (RADIO_SHORTS_READY_START_Enabled << RADIO_SHORTS_READY_START_Pos) |
        						(RADIO_SHORTS_END_DISABLE_Enabled << RADIO_SHORTS_END_DISABLE_Pos); 
        	NRF_RADIO->PACKETPTR = (uint32_t)rx_packet;

			crc = false;
			while (crc == false)
			{
				NRF_RADIO->EVENTS_DISABLED = 0;
				NRF_RADIO->TASKS_RXEN = 1;
				while (NRF_RADIO->EVENTS_DISABLED == 0)
				{
				}
				// got a packet, check for CRC
				if (NRF_RADIO->CRCSTATUS == 1)
				{
					crc = true;
				}
			}
			// we have successfully received a good packet
			// acknowledge 
			NRF_RADIO->PACKETPTR = (uint32_t)tx_packet;
			NRF_RADIO->EVENTS_DISABLED = 0;
			NRF_RADIO->TASKS_TXEN = 1;
			while (NRF_RADIO->EVENTS_DISABLED == 0)
			{
			}

			// output the results over UART
			simple_uart_putbuf(rx_packet, 19);
    }
    }
	}
}






























































