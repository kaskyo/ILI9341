#include "stm32f7xx_hal.h"

void MX_SPI5_Init() {
	bcm2835_init();
	bcm2835_spi_begin();
	
	//Set CS pins polarity to low
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, 0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, 0);

	bcm2835_spi_setClockDivider(BCM2835_SPI_CLOCK_DIVIDER_128);

	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
}

void MX_GPIO_Init() {
	bcm2835_gpio_fsel(LCD_DC_PIN, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_set_pud(LCD_DC_PIN, BCM2835_GPIO_PUD_UP);
}


void HAL_SPI_Transmit(char* data, int len, int timeout)
{
	bcm2835_spi_transfern(data, len);
}

void HAL_GPIO_WritePin(unsigned int pin, unsigned int state) {
	bcm2835_gpio_write(pin, state);	
}

void HAL_Delay(int ms) {
	clock_t start_time = clock(); 
    while (clock() < start_time + ms) ;
}
