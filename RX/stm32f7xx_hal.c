#include "stm32f7xx_hal.h"

void MX_SPI5_Init() {
	bcm2835_init();
	bcm2835_spi_begin();
	
	//Set CS pins polarity to low
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS0, 0);
	bcm2835_spi_setChipSelectPolarity(BCM2835_SPI_CS1, 0);

	bcm2835_spi_setClockDivider(6);

	bcm2835_spi_setDataMode(BCM2835_SPI_MODE0);
	bcm2835_spi_chipSelect(BCM2835_SPI_CS0);
}

void MX_GPIO_Init() {
	wiringPiSetup();
	pinMode (LCD_DC_PIN, OUTPUT);
	pinMode (LCD_RST_PIN, OUTPUT);
	pinMode (CS_Pin, OUTPUT);
}


void HAL_SPI_Transmit(char* data, int len, int timeout)
{
	bcm2835_spi_transfern(data, len);
}

void HAL_GPIO_WritePin(unsigned int pin, unsigned int state) {
	digitalWrite(pin, state);	
}

void HAL_Delay(int ms) {
	bcm2835_delay(ms);
}
