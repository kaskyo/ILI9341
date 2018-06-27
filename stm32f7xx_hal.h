#pragma once

#include "bcm2835.h"
#include <time.h>

#define	GPIO_PIN_RESET 0
#define	GPIO_PIN_SET 1
#define LCD_DC_PIN RPI_BPLUS_GPIO_J8_05
#define LCD_RST_PIN RPI_BPLUS_GPIO_J8_07

void MX_SPI5_Init();

void MX_GPIO_Init();



void HAL_SPI_Transmit(char* data, int len, int timeout);

void HAL_GPIO_WritePin(unsigned int pin, unsigned int state);

void HAL_Delay(int ms);



