#pragma once

#include <bcm2835.h>
#include <time.h>

enum {
	GPIO_PIN_RESET,
	GPIO_PIN_SET
} GPIO_State;


void MX_SPI5_Init();

void MX_GPIO_Init();



void HAL_SPI_Transmit(unsigned char* data, int len, int timeout);

void HAL_GPIO_WritePin(unsigned int pin, GPIO_State state);

void HAL_Delay(int ms);



#define DC_Pin RPI_GPIO_P1_05
#define CS_Pin 