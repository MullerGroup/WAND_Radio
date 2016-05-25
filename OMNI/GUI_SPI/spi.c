/* spi.c
 * 

main loop calls spi_write on the data buffer if there is any data to send back to the GUI
after each call to spi_write, the main loop checks a value to see if it should read from SPI
this value will be set everytime there is a turnaround

reading: 

check the MISO bit
if true, there is data to be read
	write the SPI_READ_COMMAND
	EVENTS_READY = 0
	for (i=0;i<readamount;i++)
	{
		write a 0
		wait for EVENTS_READY
		if i = 0
			read to dummy
		else
			read to the buffer
		while(MISO bit is false)
		{	
			raise SS
		}
		lower SS
	}

else, don't do anything

 * GUI-side SPI interface for transmitting and receiving data from the PC
 */

#include "spi.h"
#include "nrf_gpio.h"
#include <stdint.h>

uint32_t dummy;
bool read;

void set_read_flag(void)
{
	read = true;
}

void clear_read_flag(void)
{
	read = false;
}

bool get_read_flag(void)
{
	return read;
}

void spi_write(uint8_t buffer[], uint8_t size)
{
	uint8_t i;

	if (size > 0)
	{
		// not an empty buffer, so actually send stuff

		// enable slave
		nrf_gpio_pin_clear(SPI_SS_PIN);

		// clear ready event
		NRF_SPI0->EVENTS_READY = 0;

		// write the write command
		NRF_SPI0->TXD = SPI_WRITE_COMMAND;

		for (i=0;i<size;i++)
		{
			NRF_SPI0->TXD = (uint32_t)buffer[i];
			while (NRF_SPI0->EVENTS_READY == 0)
			{
			}
			NRF_SPI0->EVENTS_READY = 0;
			dummy = NRF_SPI0->RXD;
		}

		while (NRF_SPI0->EVENTS_READY == 0)
		{
		}
		NRF_SPI0->EVENTS_READY = 0;
		dummy = NRF_SPI0->RXD;

		// disable slave
		nrf_gpio_pin_set(SPI_SS_PIN);
	}
}

void spi_read(uint8_t buffer[], uint8_t size)
{
	uint8_t i;

	if (size > 0)
	{
		// enable slave
		nrf_gpio_pin_clear(SPI_SS_PIN);

		// clear the ready event
		NRF_SPI0->EVENTS_READY = 0;

		// write the read command
		NRF_SPI0->TXD = SPI_READ_COMMAND;

		for (i=0;i<size;i++)
		{
			NRF_SPI0->TXD = 0;
			while (NRF_SPI0->EVENTS_READY == 0)
			{
			}
			NRF_SPI0->EVENTS_READY = 0;
			if (i < 1)
			{
				dummy = NRF_SPI0->RXD;
			}
			else
			{
				buffer[i-1] = NRF_SPI0->RXD;
			}
		}

		while (NRF_SPI0->EVENTS_READY == 0)
		{
		}
		NRF_SPI0->EVENTS_READY = 0;
		buffer[size-1] = NRF_SPI0->RXD;

		// disable slave
		nrf_gpio_pin_set(SPI_SS_PIN);
	}
}

bool spi_read_with_NAK(uint8_t buffer[], uint8_t size)
{
	uint8_t i;
	bool success;
	bool reset;

	success = true;
	if (size > 0)
	{
		if (!nrf_gpio_pin_read(SPI_RX_STATUS_PIN))
		{
			// enable slave
			nrf_gpio_pin_clear(SPI_SS_PIN);

			// clear the ready event
			NRF_SPI0->EVENTS_READY = 0;

			// write the read command
			NRF_SPI0->TXD = SPI_READ_COMMAND;

			for (i=0;i<size;i++)
			{
				reset = false;
				while(!nrf_gpio_pin_read(SPI_NAK_PIN))
				{
					nrf_gpio_pin_set(SPI_SS_PIN);
					reset = true;
				}
				if (reset)
				{
					while(NRF_SPI0->EVENTS_READY == 0)
					{
					}
					NRF_SPI0->EVENTS_READY = 0;
					if (i < 1)
					{
						dummy = NRF_SPI0->RXD;
					}
					else
					{
						buffer[i-1] = NRF_SPI0->RXD;
					}
					nrf_gpio_pin_clear(SPI_SS_PIN);
					NRF_SPI0->TXD = SPI_READ_COMMAND;
				}

				NRF_SPI0->TXD = 0;
				while (NRF_SPI0->EVENTS_READY == 0)
				{
				}
				NRF_SPI0->EVENTS_READY = 0;
				if (i < 1)
				{
					dummy = NRF_SPI0->RXD;
				}
				else
				{
					buffer[i-1] = NRF_SPI0->RXD;
				}
			}

			while (NRF_SPI0->EVENTS_READY == 0)
			{
			}
			NRF_SPI0->EVENTS_READY = 0;
			buffer[size-1] = NRF_SPI0->RXD;

			// disable slave
			nrf_gpio_pin_set(SPI_SS_PIN);
		}
		else
		{
			success = false;
		}
	}
	return success;
}

void spi_write_with_NAK(uint8_t buffer[], uint8_t size)
{
	uint8_t i;
	 bool reset = false;

	if (size > 0)
	{
		// not an empty buffer, so actually send stuff

		// enable slave
		nrf_gpio_pin_clear(SPI_SS_PIN);

		// clear ready event
		NRF_SPI0->EVENTS_READY = 0;

		// write the write command
		NRF_SPI0->TXD = SPI_WRITE_COMMAND;

		for (i=0;i<size;i++)
		{

			NRF_SPI0->TXD = (uint32_t)buffer[i];
			while (NRF_SPI0->EVENTS_READY == 0)
			{
			}
			NRF_SPI0->EVENTS_READY = 0;
			dummy = NRF_SPI0->RXD;
            
            while(!nrf_gpio_pin_read(SPI_NAK_PIN))
            {
                nrf_gpio_pin_set(SPI_SS_PIN);
                while(NRF_SPI0->EVENTS_READY == 0)
                {
                    ;
                }
                NRF_SPI0->EVENTS_READY = 0;
                dummy = NRF_SPI0->RXD;
                nrf_gpio_pin_clear(SPI_SS_PIN);
                NRF_SPI0->TXD = SPI_WRITE_COMMAND;
                reset = true;
            }
            if(reset) // if the last byte sent was not received (NAK)
            {
                if (i!=0) i--; // resend it
                reset = false;
            }
            
		}
        // check to see if the last byte in a packet was written correctly
        while(!nrf_gpio_pin_read(SPI_NAK_PIN))
        {
            nrf_gpio_pin_set(SPI_SS_PIN);
            while(NRF_SPI0->EVENTS_READY == 0)
            {
                ;
            }
            NRF_SPI0->EVENTS_READY = 0;
            dummy = NRF_SPI0->RXD;
            nrf_gpio_pin_clear(SPI_SS_PIN);
            NRF_SPI0->TXD = SPI_WRITE_COMMAND;
            
            // try to write last byte again
            NRF_SPI0->TXD = (uint32_t)buffer[size-1];
            while (NRF_SPI0->EVENTS_READY == 0)
            {
            }
            NRF_SPI0->EVENTS_READY = 0;
            dummy = NRF_SPI0->RXD;
        }


		while (NRF_SPI0->EVENTS_READY == 0)
		{
		}
		NRF_SPI0->EVENTS_READY = 0;
		dummy = NRF_SPI0->RXD;

		// disable slave
		nrf_gpio_pin_set(SPI_SS_PIN);
	}
}

void spi_init(void)
{
	read = false;
	// Set up the Pins for SPI
	NRF_SPI0->PSELSCK = SPI_SCK_PIN;
	NRF_SPI0->PSELMOSI = SPI_MOSI_PIN;
	NRF_SPI0->PSELMISO = SPI_MISO_PIN;
	
	
	nrf_gpio_cfg_output(SPI_SS_PIN);
	// de-assert SS (active low)
	nrf_gpio_pin_set(SPI_SS_PIN); 

	nrf_gpio_cfg_input(SPI_NAK_PIN, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_input(SPI_RX_STATUS_PIN, NRF_GPIO_PIN_NOPULL);
	//nrf_gpio_cfg_input(SPI_TX_STATUS_PIN, NRF_GPIO_PIN_NOPULL);

	// Set SPI Frequency
	NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M2 << SPI_FREQUENCY_FREQUENCY_Pos;

	// Configure SPI
	NRF_SPI0->CONFIG = (SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos) | 
						(SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos) | 
						(SPI_CONFIG_CPOL_ActiveHigh << SPI_CONFIG_CPOL_Pos);



	// Enable the SPI0 Master Block
	NRF_SPI0->ENABLE = SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos;
}
