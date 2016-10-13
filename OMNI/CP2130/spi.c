#include "spi.h"
#include "spi_slave.h"
#include "app_error.h"
#include "boards.h"

#define PACKET_LENGTH 200

static uint8_t tx_buf[PACKET_LENGTH];
static uint8_t rx_buf[PACKET_LENGTH];

static void spi_slave_event_handle(spi_slave_evt_t event)
{
	uint32_t err_code;

	if (event.evt_type == SPI_SLAVE_XFER_DONE)
	{
		if (rx_buf[0] == 0x55)
		{
			for (int i = 0; i < PACKET_LENGTH; i++)
			{
				tx_buf[i]++;
			}
		}
		else if (rx_buf[0] == 0xAA)
		{
			for (int i = 0; i < PACKET_LENGTH; i++)
			{
				tx_buf[i]--;
			}
		}
		err_code = spi_slave_buffers_set(tx_buf, rx_buf, PACKET_LENGTH, PACKET_LENGTH);
    	APP_ERROR_CHECK(err_code);
	}
}

uint32_t spi_init(void)
{
	for (int i = 0; i < PACKET_LENGTH; i++)
	{
		tx_buf[i] = 0;
		rx_buf[i] = 0;
	}

	uint32_t 			err_code;
	spi_slave_config_t	spi_slave_config;

	err_code = spi_slave_evt_handler_register(spi_slave_event_handle);
	APP_ERROR_CHECK(err_code);

	spi_slave_config.pin_miso         = 0;
    spi_slave_config.pin_mosi         = 1;
    spi_slave_config.pin_sck          = 2;
    spi_slave_config.pin_csn          = 3;
    spi_slave_config.mode             = SPI_MODE_0;
    spi_slave_config.bit_order        = SPIM_MSB_FIRST;
    spi_slave_config.def_tx_character = 0xFF;
    spi_slave_config.orc_tx_character = 0xFF;

    err_code = spi_slave_init(&spi_slave_config);
    APP_ERROR_CHECK(err_code);

    err_code = spi_slave_buffers_set(tx_buf, rx_buf, PACKET_LENGTH, PACKET_LENGTH);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}