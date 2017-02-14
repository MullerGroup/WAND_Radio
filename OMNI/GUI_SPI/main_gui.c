#include <stdint.h>
#include <stdbool.h>
#include "radio_config.h"
#include "spi.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "nrf_gpio.h"
#include "boards.h"
#include "nrf_delay.h"
#include "app_error.h"


void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}

void init(void)
{
    /* Start 16 MHz crystal oscillator */
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART    = 1;

    /* Wait for the external oscillator to start up */
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) 
    {
    }

	const uint32_t err_code = spi_init();
    APP_ERROR_CHECK(err_code);
	radio_configure();

    __enable_irq();
}


int main(void)
{
	init();

	while (true) 
    {
        // data = read_data();
        // if (data != 0)
        // {
        //     spi_data = write_radio();
        //     if (spi_data != 0)
        //     {
        //         for(i=0;i<PACKET_SIZE+1;i++)
        //         {
        //             spi_data[i] = data[i];
        //         }
        //         finish_write_radio();
        //     }
        //     else
        //     {
        //         unread_data();
        //     }
        // }
    }
}