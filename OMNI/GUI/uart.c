/* uart.c
 * 
 * GUI-side UART interface for transmitting and receiving data from the PC
 */

#include "uart.h"
#include "command_fifo.h"
#include "radio_config.h"

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include "nrf.h"
#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "boards.h"

uint8_t command[COMMAND_SIZE];  // buffer for holding bytes of an incoming command
uint8_t c_index = 0;              // current index into command buffer
uint8_t command_history[200];
uint8_t j=0;

// transmits a single byte over UART
void simple_uart_put(uint8_t cr)
{
    NRF_UART0->TXD = (uint8_t)cr;

    while (NRF_UART0->EVENTS_TXDRDY != 1)
    {
        // Wait for TXD data to be sent.
    }

    NRF_UART0->EVENTS_TXDRDY = 0;
}

// transmits a buffer of data over UART
void simple_uart_putbuf(uint8_t buffer[], uint8_t size)
{
	uint8_t i;

	for (i=0;i<size;i++)
	{
		simple_uart_put(buffer[i]);
	}
}

// configures the uart peripheral
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

    NRF_UART0->BAUDRATE      = (UART_Baud1M << UART_BAUDRATE_BAUDRATE_Pos);
    NRF_UART0->ENABLE        = (UART_ENABLE_ENABLE_Enabled << UART_ENABLE_ENABLE_Pos);

    NRF_UART0->INTENSET = (UART_INTENSET_RXDRDY_Enabled << UART_INTENSET_RXDRDY_Pos);

    NVIC_SetPriority(UART0_IRQn, 0); //Highest priority
    NVIC_EnableIRQ(UART0_IRQn);

    NRF_UART0->TASKS_STARTTX = 1;
    NRF_UART0->TASKS_STARTRX = 1;
    NRF_UART0->EVENTS_RXDRDY = 0;


    nrf_delay_ms(3);
}

// Interrupt handler for received UART char event
// Writes char to a command buffer and loads it to fifo once full command is received
void UART0_IRQHandler(void)
{
	uint8_t i; // index for loop
	uint8_t *write_pointer;    // pointer to fifo for writing a completed command

    if ((NRF_UART0->EVENTS_RXDRDY == 1) && (NRF_UART0->INTENSET & UART_INTENSET_RXDRDY_Msk))
    {
        // UART received data interrupt

        NRF_UART0->EVENTS_RXDRDY = 0;       // clear the interrupt

        command[c_index] = NRF_UART0->RXD;    // take data and put it into the command buffer
        command_history[j] = command[c_index];
        j = (j + 1)%200;
        c_index++;                            // increment command buffer index

        if (c_index == COMMAND_SIZE)
        {
            // full command has been received, need to load it into command fifo

        	c_index = 0;         // reset command buffer index
        	write_pointer = write_command();   // get the pointer of command fifo index to write to
            if (write_pointer != 0)
            {
                // fifo is not full, so write in command
                for (i=0;i<COMMAND_SIZE;i++)
                {
                    write_pointer[i] = command[i];
                }
                finish_write_command();         // finish write and update pointers
            }
        }
    }
}












