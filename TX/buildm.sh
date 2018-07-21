gcc -c main.c
gcc bcm2835.o stm32f7xx_hal.o ILI9341_STM32_Driver.o ILI9341_GFX.o main.o -lwiringPi -ljpeg

./a.out
