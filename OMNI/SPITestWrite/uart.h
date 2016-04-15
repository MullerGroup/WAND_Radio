#ifndef UART_H
#define UART_H

#include <stdint.h>
#include <stdbool.h>

void simple_uart_put(uint8_t cr);
void simple_uart_putbuf(uint8_t buffer[], uint8_t size);
void simple_uart_putstring(const uint8_t * str);
void simple_uart_config(uint8_t rts_pin_number,
                        uint8_t txd_pin_number,
                        uint8_t cts_pin_number,
                        uint8_t rxd_pin_number,
                        bool    hwfc);

#endif