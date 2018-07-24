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

unsigned char *rgb565_buffer;


unsigned char Code(unsigned char I) {
  return I^(I<<1)^(I<<3);
} 


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
	options.c_cflag = B500000 | CS8 | CLOCAL;		//<Set baud rate
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
	unsigned char* buffer;
	unsigned char* bufferH;
	unsigned char beacon[8] = { 'P', 'e', 't', 'o', 'u', 'c', 'h', '\0' };
	unsigned char temp;
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
		uint16_t filelenH = filelen*2;		// Get the current byte offset in the file		
		rewind(jpeg);                      // Jump back to the beginning of the file

		//printf("Filelen: %d\n",filelen);
		buffer = (unsigned char *)malloc(filelen+100); // Enough memory for file + \0
		bufferH = (unsigned char *)malloc(filelenH+100);

		fread(buffer, filelen, 1, jpeg); // Read in the entire file
		fclose(jpeg); // Close the file
		for (uint16_t i=0; i<filelenH; i++)
		{	
			temp = Code(*(buffer + 4*i));
			write (bufferH; temp ; 1);
		}
		
		
		
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("File read (in %f ms)\n\n",timespent);
		//char lol = 'U';
		//fwrite((const void*) &lol,1,1,uart);
		
		//begin = clock();
		write(uart0_filestream, (const void*) &beacon,8);
		//HAL_Delay(500);
		write(uart0_filestream, (const void*) &filelenH,sizeof(uint16_t));
		FILE* out = fopen ("tx.jpg","wb");
		uint16_t i=0, wrl;
		while (i<filelenH)
		{
			
			wrl = write(uart0_filestream, bufferH + i,filelenH - i);
			fwrite(bufferH + i, sizeof(char),wrl,out);
			i += wrl;
		}
		fclose(out);
		//end = clock();
		//timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("Sent. (in %f ms)\n\n",timespent);
		free(buffer);
		free(bufferH);
		
	}
	free(rgb565_buffer);
	//fclose(uart);
}
