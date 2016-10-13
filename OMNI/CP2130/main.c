#include "spi.h"
#include "app_error.h"
#include "boards.h"
#include <stdint.h>

void app_error_handler(uint32_t error_code, uint32_t line_num, const uint8_t * p_file_name)
{
    for (;;)
    {
        // No implementation needed.
    }
}

int main(void)
{
	const uint32_t err_code = spi_init();
	APP_ERROR_CHECK(err_code);

	while(true) {}
}