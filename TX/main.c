#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "stdio.h"
#include "snow_tiger.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

#include <stdlib.h>
#include <termios.h>		//Used for UART
#include <time.h>
#include <string.h>

#include "RS152.h"

#define BAUDRATE B1000000

int uart0_filestream = -1;
void InitUART()
{
	uart0_filestream = open("/dev/serial0", O_WRONLY | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);	
}


int main()
{/*
	//system("stty -F /dev/serial0 2000000");
*/	//FILE* uart = fopen("/dev/serial0","wb");
	InitUART();
	GenerateInttables();
	FILE* jpeg;
	uint8_t* buffer;
	uint64_t* bufferRS;
	
	uint8_t beacon[8] = { 'P', 'e', 't', 'o', 'u', 'c', 'h', '\0' };
	for (;;) {

		system("/home/pi/ILI9341/cam/do_caputure.sh");
		//clock_t end = clock();
		//double timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("(in %f ms)\n\n",timespent);
		HAL_Delay(168);
		
		
		//begin = clock();
		jpeg = fopen("/home/pi/ILI9341/1.jpg","rb");
		fseek(jpeg, 0, SEEK_END);          // Jump to the end of the file
		
		uint16_t filelen = ftell(jpeg);
		
		rewind(jpeg);                      // Jump back to the beginning of the file

		//printf("Filelen: %d\n",filelen);
		buffer = (uint8_t*)malloc(filelen+100); // Enough memory for file + \0
		bufferRS = (uint64_t*)malloc(filelen*8+96);
		fread(buffer, filelen, 1, jpeg); // Read in the entire file
		fclose(jpeg); // Close the file
			for (uint16_t i=0; i<filelen; i++)
			{	
				bufferRS[i] = RS152Code(buffer[i]);
			}
		
		
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("File read (in %f ms)\n\n",timespent);
		//char lol = 'U';
		//fwrite((const void*) &lol,1,1,uart);
		
		//begin = clock();
		
		// Encode beacon too
		for (int i=0; i<8; i++)
			write(uart0_filestream, (const void*) RS152Code(beacon[i]),8);
		//HAL_Delay(500);
		
		write(uart0_filestream, (const void*) RS152Code(filelen&0xff),8);
		write(uart0_filestream, (const void*) RS152Code((filelen&0xff00)>>8),8);
		
		FILE* out = fopen ("tx.jpg","wb");
		uint16_t j=0, wrl;
		
		uint8_t* buf1 = (uint8_t*)bufferRS;
		
			while (j<filelen*8)
			{
				wrl = write(uart0_filestream, buf1 + j,filelen*8 - j);
				//fwrite(bufferH + j, sizeof(uint8_t),wrl,out);
				j += wrl;
			}
		fclose(out);
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("Sent. (in %f ms)\n\n",timespent);
		buf1 = NULL;
		free(buffer);
		free(bufferRS);
	}
	//fclose(uart);
}
