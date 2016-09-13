/* spi.h
 * 
 * GUI-side SPI interface for transmitting and receiving data from the PC
 */

#include <stdint.h>
#include <stdbool.h>


#define SPI_SCK_PIN			0
#define SPI_MOSI_PIN		2
#define SPI_MISO_PIN		3
#define SPI_SS_PIN			1
#define SPI_NAK_PIN			4
#define SPI_RX_STATUS_PIN	4
#define SPI_TX_STATUS_PIN	6

#define SPI_WRITE_COMMAND 	0x00
// #define SPI_READ_COMMAND	0x40
#define SPI_READ_COMMAND	0x02

#define SPI_FREQUENCY_FREQUENCY_M3 0x30000000
#define SPI_FREQUENCY_FREQUENCY_M2_5 0x28000000

void set_read_flag(void);
void clear_read_flag(void);
bool get_read_flag(void);
void spi_write(uint8_t buffer[], uint8_t size);
void spi_read(uint8_t buffer[], uint8_t size);
bool spi_read_with_NAK(uint8_t buffer[], uint8_t size);
void spi_write_with_NAK(uint8_t buffer[], uint8_t size);
void spi_init(void);

