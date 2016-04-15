/* uart.h
 * 
 * GUI-side UART interface for transmitting and receiving data from the PC
 */

#ifndef UART_H
#define UART_H

// Baudrate definitions

#define UART_Baud1200 (0x0004F000UL) /*!< 1200 baud. */
#define UART_Baud2400 (0x0009D000UL) /*!< 2400 baud. */
#define UART_Baud4800 (0x0013B000UL) /*!< 4800 baud. */
#define UART_Baud9600 (0x00275000UL) /*!< 9600 baud. */
#define UART_Baud14400 (0x003B0000UL) /*!< 14400 baud. */
#define UART_Baud19200 (0x004EA000UL) /*!< 19200 baud. */
#define UART_Baud28800 (0x0075F000UL) /*!< 28800 baud. */
#define UART_Baud38400 (0x009D5000UL) /*!< 38400 baud. */
#define UART_Baud57600 (0x00EBF000UL) /*!< 57600 baud. */
#define UART_Baud76800 (0x013A9000UL) /*!< 76800 baud. */
#define UART_Baud115200 (0x01D7E000UL) /*!< 115200 baud. */
#define UART_Baud230400 (0x03AFB000UL) /*!< 230400 baud. */
#define UART_Baud250000 (0x04000000UL) /*!< 250000 baud. */
#define UART_Baud460800 (0x075F7000UL) /*!< 460800 baud. */
#define UART_Baud921600 (0x0EBEDFA4UL) /*!< 921600 baud. */
#define UART_Baud1M (0x10000000UL) /*!< 1M baud. */

// not necessarily supported
#define UART_Baud2M (0x20000000UL)

#include "radio_config.h"
#include <stdint.h>
#include <stdbool.h>

void simple_uart_put(uint8_t cr);						// transmits char over uart

void simple_uart_putbuf(uint8_t buffer[], uint8_t size);	// transmits buffer of chars

void simple_uart_config(uint8_t rts_pin_number,			// configures uart channel
                        uint8_t txd_pin_number,
                        uint8_t cts_pin_number,
                        uint8_t rxd_pin_number,
                        bool    hwfc);

#endif

