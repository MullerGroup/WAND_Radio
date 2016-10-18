/* spi.c
 * 
 * SPI interface with GUI.
 */

#include "spi.h"
#include "spi_slave.h"
#include "command_fifo.h"
#include "data_fifo.h"
#include "radio_fifo.h"
#include "boards.h"
#include "app_error.h"
#include "radio_config.h"
#include "nrf_gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include "nrf_delay.h"

static uint8_t empty_write_buf[SPI_WRITE_LENGTH];
static uint8_t full_read_buf[COMMAND_SIZE];

uint8_t *tx_buf;
uint8_t *rx_buf;
bool spi_running;
int spi_count = 0;

void spi_slave_event_handle(spi_slave_evt_t event)
{
	uint32_t err_code;

	if (event.evt_type == SPI_SLAVE_XFER_DONE)
	{
		// nrf_gpio_pin_set(SPI_RTR_PIN);
		if (event.tx_amount == COMMAND_SIZE)
		{
			// GUI was sending a command
			if (rx_buf != full_read_buf)
			{
				// we received a command and it is now in the command fifo
				finish_write_command();
			}
			rx_buf = write_command();
			if (rx_buf == 0)
			{
				rx_buf = full_read_buf;
			}
		}
		else if(event.tx_amount == SPI_WRITE_LENGTH)
		{
			spi_count++;
			// GUI was reading back data (or registers, but same number of actual bytes)
			// we know that we just successfully sent out a data packet, so get the next one
			// if (tx_buf != empty_write_buf)
			// {
			// 	finish_read_radio();
			// }

			tx_buf = read_data();
			if (tx_buf == 0)
			{
				tx_buf = empty_write_buf;
				// nrf_delay_us(500);
				// nrf_gpio_pin_set(SPI_RTR_PIN);
				// spi_running = false;
			}
			// check if we can write command
			if (rx_buf == full_read_buf)
			{
				// command fifo was previously full, should try again
				rx_buf = write_command();
				if (rx_buf == 0)
				{
					rx_buf = full_read_buf;
				}
			}
		}
		err_code = spi_slave_buffers_set(tx_buf, rx_buf, SPI_WRITE_LENGTH, COMMAND_SIZE);
		APP_ERROR_CHECK(err_code);
	}
	// else if (event.evt_type == SPI_SLAVE_BUFFERS_SET_DONE)
	// {
	// 	if (spi_running)
	// 	{
	// 		nrf_gpio_pin_clear(SPI_RTR_PIN);
	// 	}
	// }
}

uint32_t spi_init(void)
{
	uint32_t err_code;
	int i;

	spi_running = false;

	spi_slave_config_t spi_slave_config;

	err_code = spi_slave_evt_handler_register(spi_slave_event_handle);
	APP_ERROR_CHECK(err_code);

	spi_slave_config.pin_miso         = SPI_MISO_PIN;
    spi_slave_config.pin_mosi         = SPI_MOSI_PIN;
    spi_slave_config.pin_sck          = SPI_SCK_PIN;
    spi_slave_config.pin_csn          = SPI_CSN_PIN;

    spi_slave_config.mode             = SPI_MODE_0;
    spi_slave_config.bit_order        = SPIM_MSB_FIRST;
    spi_slave_config.def_tx_character = DEF_CHARACTER;
    spi_slave_config.orc_tx_character = ORC_CHARACTER;

    err_code = spi_slave_init(&spi_slave_config);
    APP_ERROR_CHECK(err_code);

    // configure and set the rtr pin high (active low)
    nrf_gpio_cfg_output(SPI_RTR_PIN);
    nrf_gpio_pin_set(SPI_RTR_PIN);

    // initialize first set of buffers
    for(i=0;i<SPI_WRITE_LENGTH;i++)
    {
    	empty_write_buf[i] = 0;
    }

    for(i=0;i<COMMAND_SIZE;i++)
    {
    	full_read_buf[i] = 0;
    }

    rx_buf = write_command();
    if (rx_buf == 0)
    {
    	rx_buf = full_read_buf;
    }
    tx_buf = empty_write_buf;

    err_code = spi_slave_buffers_set(tx_buf, rx_buf, SPI_WRITE_LENGTH, COMMAND_SIZE);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

void radio_spi_start(void)
{
	uint32_t err_code;
	uint8_t *tx_try;

	if (!spi_running)
	{
		tx_try = read_data();
		if (tx_try != 0)
		{
			spi_running = true;
			err_code = spi_slave_buffers_set(tx_try, rx_buf, SPI_WRITE_LENGTH, COMMAND_SIZE);
			if (err_code == NRF_SUCCESS)
			{
				// we successfully set buffers, ready to start SPI.
				//spi_running = true;
				//nrf_gpio_pin_clear(SPI_RTR_PIN);
				tx_buf = tx_try;
			}
			else
			{
				// need to un-read that data point...
				unread_data();
				spi_running = false;
				//nrf_gpio_pin_set(SPI_RTR_PIN);
			}
		}
	}
}
