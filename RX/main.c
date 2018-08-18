#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "stdio.h"
#include "snow_tiger.h"

#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <sys/stat.h>

#include <jpeglib.h>
#include <stdlib.h>
#include <termios.h>		//Used for UART
#include <time.h>
#include <string.h>
#include <setjmp.h>

#include "RS152.h"

#define BAUDRATE B1000000
//#define ham


struct jpegErrorManager {
    struct jpeg_error_mgr pub;
    jmp_buf setjmp_buffer;
};
char jpegLastErrorMsg[JMSG_LENGTH_MAX];
void jpegErrorExit (j_common_ptr cinfo)
{
    struct jpegErrorManager* myerr = (struct jpegErrorManager*)cinfo->err;
    ( *(cinfo->err->format_message) ) (cinfo, jpegLastErrorMsg);
    longjmp(myerr->setjmp_buffer, 1);
  
}


unsigned char *rgb565_buffer;

int readJPG(char* fileName) {
	
	int rc, i, j;
	unsigned long jpg_size;
	unsigned char *jpg_buffer;
	struct stat file_info;
	struct jpeg_decompress_struct cinfo;
	struct jpegErrorManager jerr;
	unsigned long bmp_size;
	unsigned char *bmp_buffer;
	int row_stride, width, height, pixel_size;

	rc = stat(fileName, &file_info);
	if (rc) {
		syslog(LOG_ERR, "FAILED to stat source jpg");
		exit(EXIT_FAILURE);
	}
	jpg_size = file_info.st_size;
	jpg_buffer = (unsigned char*) malloc(jpg_size + 100);

	int fd = open(fileName, O_RDONLY);
	i = 0;
	while (i < jpg_size) {
		rc = read(fd, jpg_buffer + i, jpg_size - i);
		syslog(LOG_INFO, "Input: Read %d/%lu bytes", rc, jpg_size-i);
		i += rc;
	}
	close(fd);
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = jpegErrorExit;
	
	if (setjmp(jerr.setjmp_buffer)) {
		printf("%s\n", jpegLastErrorMsg);
		jpeg_destroy_decompress(&cinfo);
		
		return 1;
	}
	
	jpeg_create_decompress(&cinfo);
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);
	rc = jpeg_read_header(&cinfo, TRUE);

	if (rc != 1) {
		exit(EXIT_FAILURE);
	}

	jpeg_start_decompress(&cinfo);
	width = cinfo.output_width;
	height = cinfo.output_height;
	pixel_size = cinfo.output_components;
	bmp_size = width * height * pixel_size;
	bmp_buffer = (unsigned char*) malloc(bmp_size);
	row_stride = width * pixel_size;

	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}
	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress(&cinfo);
	free(jpg_buffer);
		
	unsigned char r,g,b,r5,g6,b5,bt1,bt2;

	for (int i=0; i<width*height; i++)
	{
		r = bmp_buffer[i*3];
		g = bmp_buffer[i*3+1];
		b = bmp_buffer[i*3+2];
		r5 = (unsigned char)((uint16_t)r*31/255);
		g6 = (unsigned char)((uint16_t)g*63/255);
		b5 = (unsigned char)((uint16_t)b*32/255);
		
		bt1 = (r5 << 3) | ((g6 >> 3) & 0x07);
		bt2 = ((g6 << 5) & 0xE0) | (b5 & 0x1F);
		
		rgb565_buffer[i*2] = bt1;
		rgb565_buffer[i*2+1] = bt2;
	}
	free(bmp_buffer);
	
	return 0;
}

int uart0_filestream = -1;
void InitUART()
{
	uart0_filestream = open("/dev/serial0", O_RDONLY | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);	
}

int main()
{
	printf("Initing...");
	ILI9341_Init();
	printf("Done\n");

	printf("Filling...");
	ILI9341_Draw_Image((const char*)snow_tiger,SCREEN_VERTICAL_2);
//	ILI9341_Fill_Screen(RED);
	printf("Done\n");
	
	rgb565_buffer = (unsigned char*)malloc(320*240*2);
	//system("stty -F /dev/serial0 2000000");
	//FILE* uart = fopen("/dev/serial0","wb");
	InitUART();
	FILE* jpeg;
	char* buffer;
	uint8_t beacon[8] = { 'P', 'e', 't', 'o', 'u', 'c', 'h', '\0' };
	uint8_t receivedBeacon[64] = {0};
	uint8_t decodedBeacon[8] = {0};
	uint16_t jpegSize;
	uint8_t jpegSizeRS[8];
	uint8_t buf;
	uint8_t jpegBufferRS[8];
	unsigned char* jpegBuffer;
	jpegBuffer = (unsigned char*)malloc(sizeof(unsigned char)*0x10000);
	unsigned char* rx;
	rx = (unsigned char*)malloc(sizeof(unsigned char));
	uint16_t rxl;
	for (;;) 
	{
		
		rxl = read(uart0_filestream, (void*)rx, 1);
		
		for (int i=0; i<63; i++)
		{
			receivedBeacon[i] = receivedBeacon[i+1];
			//printf("%hhX",receivedBeacon[i]);
		}
		receivedBeacon[63] = rx;
		
		for (int i=0; i<8; i++) {
			decodedBeacon[i] = RS152Decode(&receivedBeacon[i*8]);
		}
		
		//printf("%hhX\n",rx);
		
		if (memcmp(beacon,decodedBeacon,8)==0)
		{
			printf("I GOT A BEACON\n");
			for (uint8_t k = 0; k < 2; k++)
			{				
				read(uart0_filestream, (void*)&jpegSizeRS, 8);
				jpegSize |= RS152Decode(jpegSizeRS) << (8 * k);
			}
			//printf("Read %d bytes\n",jpegSize);
			printf("Size: %d\n", jpegSize);
			uint16_t i = 0;
			FILE* fp=fopen("rx.jpg", "wb");
			while (i < jpegSize) 
			{
				read(uart0_filestream, &jpegBufferRS, 8);
				buf = RS152Decode(jpegBufferRS);
				i++;
				fwrite(&buf, sizeof(char), 1, fp);
			}
			fclose(fp);
			//rxl = read(uart0_filestream, (void*)jpegBuffer, jpegSize);
			printf("Read %d bytes\n",i);
			if (jpegSize>0) {
				if (readJPG("rx.jpg") == 0)
					ILI9341_Draw_Image((const char*)rgb565_buffer,SCREEN_HORIZONTAL_1);
				
			}
			
		}
	jpegSize = 0;
	}
	free(rgb565_buffer);
	//fclose(uart);
}
