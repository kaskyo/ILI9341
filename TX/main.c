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

#define BAUDRATE B500000
//#define ham

unsigned char *rgb565_buffer;

#ifdef ham
uint8_t Code(uint8_t I) {
  return I^(I<<1)^(I<<3);
} 

uint16_t Code8 (uint8_t G)
{
	uint8_t t1 = Code ( G & 0x0f);
	uint8_t t2 = Code ( G >> 4 & 0x0f);
	uint16_t t = t1 | (t2 << 8);
	return t;
}
#endif

void readJPG(char* fn) {
	
	int rc, i, j;
	struct stat file_info;
	unsigned long jpg_size;
	unsigned char *jpg_buffer;
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int row_stride, width, height, pixel_size;
	unsigned long bmp_size;
	unsigned char *bmp_buffer;

	rc = stat(fn, &file_info);
	if (rc) {
		exit(EXIT_FAILURE);
	}
	jpg_size = file_info.st_size;
	jpg_buffer = (unsigned char*) malloc(jpg_size + 100);

	int fd = open(fn, O_RDONLY);
	i = 0;
	while (i < jpg_size) {
		rc = read(fd, jpg_buffer + i, jpg_size - i);
		i += rc;
	}
	close(fd);

	cinfo.err = jpeg_std_error(&jerr);	
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

	
	for (uint32_t i=0; i<width*height; i++)
	{
		r = bmp_buffer[i*3];
		g = bmp_buffer[i*3+1];
		b = bmp_buffer[i*3+2];
		r5 = (uint8_t)((uint16_t)r*31/255);
		g6 = (uint8_t)((uint16_t)g*63/255);
		b5 = (uint8_t)((uint16_t)b*32/255);
		
		bt1 = (r5 << 3) | ((g6 >> 3) & 0x07);
		bt2 = ((g6 << 5) & 0xE0) | (b5 & 0x1F);
		
		rgb565_buffer[i*2] = bt1;
		rgb565_buffer[i*2+1] = bt2;
	}

	free(bmp_buffer);
}

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
	FILE* jpeg;
	uint8_t* buffer;
	#ifdef ham
	uint8_t* bufferH;
	uint8_t temp1, temp2;
	#endif
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
		#ifdef ham
			uint16_t filelenH = filelen*2;		// Get the current byte offset in the file		
			uint32_t filelenHx = 0;
		#endif
		rewind(jpeg);                      // Jump back to the beginning of the file

		//printf("Filelen: %d\n",filelen);
		buffer = (unsigned char *)malloc(filelen+100); // Enough memory for file + \0
		#ifdef ham
			bufferH = (unsigned char *)malloc(filelenH+100);
		#endif
		fread(buffer, filelen, 1, jpeg); // Read in the entire file
		fclose(jpeg); // Close the file
		#ifdef ham
			for (uint16_t i=0; i<filelen; i++)
			{	
				temp1 = Code(buffer[i]&0x0f);
				temp2 = Code((buffer[i]&0xf0)>>4);
				bufferH[i*2] = temp1;
				bufferH[i*2+1] = temp2;
			}
		#else
			
		#endif
		
		
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("File read (in %f ms)\n\n",timespent);
		//char lol = 'U';
		//fwrite((const void*) &lol,1,1,uart);
		
		//begin = clock();
		write(uart0_filestream, (const void*) &beacon,8);
		//HAL_Delay(500);
		#ifdef ham
		for (uint8_t h = 0; h < 16; h=h+8)
		{
			filelenHx = Code8((filelenH >> h));
			write(uart0_filestream, (const void*) &filelenHx,sizeof(uint16_t));
		}
		#else
			write(uart0_filestream, (const void*) &filelen,sizeof(uint16_t));
		#endif
		FILE* out = fopen ("tx.jpg","wb");
		uint16_t j=0, wrl;
		#ifdef ham
			while (j<filelenH)
			{
				wrl = write(uart0_filestream, bufferH + j,filelenH - j);
				fwrite(bufferH + j, sizeof(uint8_t),wrl,out);
				j += wrl;
			}
		#else
			while (j<filelen)
			{
				wrl = write(uart0_filestream, buffer + j,filelen - j);
				fwrite(buffer + j, sizeof(uint8_t),wrl,out);
				j += wrl;
			}
		#endif
		fclose(out);
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("Sent. (in %f ms)\n\n",timespent);
		free(buffer);
		#ifdef ham
			free(bufferH);
		#endif
	}
	free(rgb565_buffer);
	//fclose(uart);
}
