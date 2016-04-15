#include "uart.h"
#include "radio_config.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

int mode = 1;
int debug = 0;

void uart_rssi(uint32_t rssival)
{
	int rssi;
	char buffer[3];
	int x;

	if (mode == 3)
	{
		buffer[0] = 0;
		buffer[1] = 0;
		buffer[2] = 0;

		rssi = rssival;
		simple_uart_putstring((const uint8_t *)"-");
		snprintf(buffer, 3, "%d", rssi);
		for (x=0;x<3;x++)
		{
			simple_uart_put((uint8_t)buffer[x]);
		}
		simple_uart_putstring((const uint8_t *)"\n\r");
	}
}

void send_uart(uint32_t counts, uint32_t errors, int arr[])
{
	int milli;
	int bitrate;
	char buffer[9];
	int x;
	int output;

    int i;
    int y;
    char buf2[5];

	if ((mode == 1) | (mode == 2))
	{
		milli = counts/16000;
		bitrate = 1000*NUM_PACKETS*PACKET_SIZE*8/milli;

		if (mode == 1){
			output = bitrate;
		}
		if (mode == 2){
			output = errors;
		}

		buffer[0] = 0;
		buffer[1] = 0;
		buffer[2] = 0;
		buffer[3] = 0;
		buffer[4] = 0;
		buffer[5] = 0;
		buffer[6] = 0;
		buffer[7] = 0;
		buffer[8] = 0;

		snprintf(buffer, 9, "%d", output);
		for (x=0;x<9;x++)
		{
			simple_uart_put((uint8_t)buffer[x]);
		}

        if ((mode == 2) & (debug == 1))
        {
            simple_uart_putstring((const uint8_t *)" errors");
        }
		simple_uart_putstring((const uint8_t *)"\n\r");

        if ((mode == 2) & (debug == 1) & (errors > 0))
        {
            if (errors > 200)
            {  
                y = 200;
            }
            else
            {
                y = errors;
            }
            for (i=0;i<y;i++)
            {
                buf2[0] = 0;
                buf2[1] = 0;
                buf2[2] = 0;
                buf2[3] = 0;
                buf2[4] = 0;
                snprintf(buf2, 5, "%d", arr[i]);
                for (x=0;x<5;x++)
                {
                    simple_uart_put((uint8_t)buf2[x]);
                }
                simple_uart_putstring((const uint8_t *)"\n\r");
            }
        }
	}
}

void simple_uart_put(uint8_t cr)
{
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
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
    NVIC_EnableIRQ(UART0_IRQn);

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;

    nrf_delay_ms(3);
}

void UART0_IRQHandler(void)
{
    uint8_t received_char;

    if ((NRF_UART0->EVENTS_RXDRDY == 1) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
    {
        NRF_UART0->EVENTS_RXDRDY = 0;
        received_char = NRF_UART0->RXD;

        if (received_char == '1')
        {    
            mode = 1;
            debug = 0;
        }
        if (received_char == '2')
        {
            mode = 2;
            debug = 0;
        }
        if (received_char == '3')
        {
            mode = 3;
            debug = 0;
        }
        if (received_char == '4')
        {
            mode = 2;
            debug = 1;
        }
        
        //nrf_gpio_pin_toggle(LED_RGB_RED);
    }
}
