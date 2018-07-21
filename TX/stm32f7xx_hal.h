#pragma once

#include "bcm2835.h"
#include <time.h>
#include <wiringPi.h>

#define	GPIO_PIN_RESET LOW
#define	GPIO_PIN_SET HIGH
#define LCD_DC_PIN 0
#define LCD_RST_PIN 1
//#define LCD_CS_PIN 2
#define CS_Pin 2

void MX_SPI5_Init();

void MX_GPIO_Init();



void HAL_SPI_Transmit(char* data, int len, int timeout);

void HAL_GPIO_WritePin(unsigned int pin, unsigned int state);

void HAL_Delay(int ms);



