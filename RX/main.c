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

unsigned char *rgb565_buffer;

void readJPG(uint16_t jpg_size, unsigned char *jpg_buffer) {
	
	int rc, i, j;

	//char *syslog_prefix = (char*) malloc(1024);
	//sprintf(syslog_prefix, "%s", fn);
	//openlog(syslog_prefix, LOG_PERROR | LOG_PID, LOG_USER);

	/*if (argc != 2) {
		fprintf(stderr, "USAGE: %s filename.jpg\n", fn);
		exit(EXIT_FAILURE);
	}*/

	// Variables for the source jpg
	struct stat file_info;

	// Variables for the decompressor itself
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;

	// Variables for the output buffer, and how long each row is

	int row_stride, width, height, pixel_size;
	unsigned long bmp_size;
	unsigned char *bmp_buffer;

	// Load the jpeg data from a file into a memory buffer for 
	// the purpose of this demonstration.
	// Normally, if it's a file, you'd use jpeg_stdio_src, but just
	// imagine that this was instead being downloaded from the Internet
	// or otherwise not coming from disk
	/*rc = stat(fn, &file_info);
	if (rc) {
		//syslog(LOG_ERR, "FAILED to stat source jpg");
		exit(EXIT_FAILURE);
	}
	jpg_size = file_info.st_size;
	jpg_buffer = (unsigned char*) malloc(jpg_size + 100);

	int fd = open(fn, O_RDONLY);
	i = 0;
	while (i < jpg_size) {
		rc = read(fd, jpg_buffer + i, jpg_size - i);
		//syslog(LOG_INFO, "Input: Read %d/%lu bytes", rc, jpg_size-i);
		i += rc;
	}
	close(fd);*/

//   SSS    TTTTTTT     A     RRRR     TTTTTTT
// SS   SS     T       A A    R   RR      T   
// S           T      A   A   R    RR     T   
// SS          T     A     A  R   RR      T   
//   SSS       T     AAAAAAA  RRRR        T   
//      SS     T     A     A  R RR        T   
//       S     T     A     A  R   R       T   
// SS   SS     T     A     A  R    R      T   
//   SSS       T     A     A  R     R     T   

	//syslog(LOG_INFO, "Proc: Create Decompress struct");
	// Allocate a new decompress struct, with the default error handler.
	// The default error handler will exit() on pretty much any issue,
	// so it's likely you'll want to replace it or supplement it with
	// your own.
	cinfo.err = jpeg_std_error(&jerr);	
	//printf("cinfo.err = jpeg_std_error(&jerr);	\n");
	jpeg_create_decompress(&cinfo);
	//printf("jpeg_create_decompress(&cinfo);\n");


	//syslog(LOG_INFO, "Proc: Set memory buffer as source");
	// Configure this decompressor to read its data from a memory 
	// buffer starting at unsigned char *jpg_buffer, which is jpg_size
	// long, and which must contain a complete jpg already.
	//
	// If you need something fancier than this, you must write your 
	// own data source manager, which shouldn't be too hard if you know
	// what it is you need it to do. See jpeg-8d/jdatasrc.c for the 
	// implementation of the standard jpeg_mem_src and jpeg_stdio_src 
	// managers as examples to work from.
	jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);
	//printf("jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);\n");


	//syslog(LOG_INFO, "Proc: Read the JPEG header");
	// Have the decompressor scan the jpeg header. This won't populate
	// the cinfo struct output fields, but will indicate if the
	// jpeg is valid.
	rc = jpeg_read_header(&cinfo, TRUE);
	//printf("rc = jpeg_read_header(&cinfo, TRUE);\n");

	if (rc != 1) {
		//syslog(LOG_ERR, "File does not seem to be a normal JPEG");
		exit(EXIT_FAILURE);
	}

	//syslog(LOG_INFO, "Proc: Initiate JPEG decompression");
	// By calling jpeg_start_decompress, you populate cinfo
	// and can then allocate your output bitmap buffers for
	// each scanline.
	jpeg_start_decompress(&cinfo);
	//printf("jpeg_start_decompress(&cinfo);\n");
	
	width = cinfo.output_width;
	//printf("width = cinfo.output_width;\n");
	//printf("width = %d\n",width);
	height = cinfo.output_height;
	//printf("height = cinfo.output_height;\n");
	//printf("height = %d\n", height);
	pixel_size = cinfo.output_components;
	//printf("pixel_size = cinfo.output_components;\n");
	//printf("pixel_size = %d\n", pixel_size);

	//syslog(LOG_INFO, "Proc: Image is %d by %d with %d components", 
	//		width, height, pixel_size);

	bmp_size = width * height * pixel_size;
	//printf("bmp_size = width * height * pixel_size;\n");
	bmp_buffer = (unsigned char*) malloc(bmp_size);
	//printf("bmp_buffer = (unsigned char*) malloc(bmp_size);\n");

	// The row_stride is the total number of bytes it takes to store an
	// entire scanline (row). 
	row_stride = width * pixel_size;
	//printf("row_stride = width * pixel_size;\n");


	//syslog(LOG_INFO, "Proc: Start reading scanlines");
	//
	// Now that you have the decompressor entirely configured, it's time
	// to read out all of the scanlines of the jpeg.
	//
	// By default, scanlines will come out in RGBRGBRGB...  order, 
	// but this can be changed by setting cinfo.out_color_space
	//
	// jpeg_read_scanlines takes an array of buffers, one for each scanline.
	// Even if you give it a complete set of buffers for the whole image,
	// it will only ever decompress a few lines at a time. For best 
	// performance, you should pass it an array with cinfo.rec_outbuf_height
	// scanline buffers. rec_outbuf_height is typically 1, 2, or 4, and 
	// at the default high quality decompression setting is always 1.
	while (cinfo.output_scanline < cinfo.output_height) {
		unsigned char *buffer_array[1];
		buffer_array[0] = bmp_buffer + \
						   (cinfo.output_scanline) * row_stride;

		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}	
	//printf("Done reading scanlines\n");
	//syslog(LOG_INFO, "Proc: Done reading scanlines");


	// Once done reading *all* scanlines, release all internal buffers,
	// etc by calling jpeg_finish_decompress. This lets you go back and
	// reuse the same cinfo object with the same settings, if you
	// want to decompress several jpegs in a row.
	//
	// If you didn't read all the scanlines, but want to stop early,
	// you instead need to call jpeg_abort_decompress(&cinfo)
	jpeg_finish_decompress(&cinfo);
	//printf("jpeg_finish_decompress(&cinfo);\n");

	// At this point, optionally go back and either load a new jpg into
	// the jpg_buffer, or define a new jpeg_mem_src, and then start 
	// another decompress operation.
	
	// Once you're really really done, destroy the object to free everything
	jpeg_destroy_decompress(&cinfo);
	//printf("jpeg_finish_decompress(&cinfo);\n");
	// And free the input buffer
	//free(jpg_buffer);
	
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
	//printf("Done BMP565\n");
	free(bmp_buffer);
	//printf("free(bmp_buffer);\n");
}

int uart0_filestream = -1;
void InitUART()
{
	
	//OPEN THE UART
	//The flags (defined in fcntl.h):
	//	Access modes (use 1 of these):
	//		O_RDONLY - Open for reading only.
	//		O_RDWR - Open for reading and writing.
	//		O_WRONLY - Open for writing only.
	//
	//	O_NDELAY / O_NONBLOCK (same function) - Enables nonblocking mode. When set read requests on the file can return immediately with a failure status
	//											if there is no input immediately available (instead of blocking). Likewise, write requests can also return
	//											immediately with a failure status if the output can't be written immediately.
	//
	//	O_NOCTTY - When set and path identifies a terminal device, open() shall not cause the terminal device to become the controlling terminal for the process.
	uart0_filestream = open("/dev/serial0", O_RDONLY | O_NOCTTY);		//Open in non blocking read/write mode
	if (uart0_filestream == -1)
	{
		//ERROR - CAN'T OPEN SERIAL PORT
		printf("Error - Unable to open UART.  Ensure it is not in use by another application\n");
	}
	
	//CONFIGURE THE UART
	//The flags (defined in /usr/include/termios.h - see http://pubs.opengroup.org/onlinepubs/007908799/xsh/termios.h.html):
	//	Baud rate:- B1200, B2400, B4800, B9600, B19200, B38400, B57600, B115200, B230400, B460800, B500000, B576000, B921600, B1000000, B1152000, B1500000, B2000000, B2500000, B3000000, B3500000, B4000000
	//	CSIZE:- CS5, CS6, CS7, CS8
	//	CLOCAL - Ignore modem status lines
	//	CREAD - Enable receiver
	//	IGNPAR = Ignore characters with parity errors
	//	ICRNL - Map CR to NL on input (Use for ASCII comms where you want to auto correct end of line characters - don't use for bianry comms!)
	//	PARENB - Parity enable
	//	PARODD - Odd parity (else even)
	struct termios options;
	tcgetattr(uart0_filestream, &options);
	options.c_cflag = B500000 | CS8 | CLOCAL | CREAD;		//<Set baud rate
	options.c_iflag = IGNPAR;
	options.c_oflag = 0;
	options.c_lflag = 0;
	tcflush(uart0_filestream, TCIFLUSH);
	tcsetattr(uart0_filestream, TCSANOW, &options);	
}

/*
int rx_length = read(uart0_filestream, (void*)rx_buffer, rx_length);
*/
int main()
{
	printf("Initing...");
	ILI9341_Init();
	printf("Done\n");
	//HAL_Delay(500);

	printf("Filling...");
	ILI9341_Fill_Screen(WHITE);
	printf("Done\n");
	//HAL_Delay(500);
	
	rgb565_buffer = (unsigned char*)malloc(320*240*2);
	/*printf("Buffer allocated\n");
	//HAL_Delay(500);
	//system("stty -F /dev/serial0 2000000");
	HAL_Delay(1000);
	*///FILE* uart = fopen("/dev/serial0","wb");
	InitUART();
	FILE* jpeg;
	char* buffer;
	unsigned char beacon[8] = { 'P', 'e', 't', 'o', 'u', 'c', 'h', '\0' };
	unsigned char receivedBeacon[8] = {0};
	
	uint16_t jpegSize;
	unsigned char* jpegBuffer;
	jpegBuffer = (unsigned char*)malloc(0x7FFF);
	unsigned char rx;
	uint16_t rxl;
	for (;;) 
	{
		
		rxl = read(uart0_filestream, (void*)&rx, 1);
		
		for (int i=0; i<7; i++)
		{
			receivedBeacon[i] = receivedBeacon[i+1];
			//printf("%hhX",receivedBeacon[i]);
		}
		receivedBeacon[7] = rx;
		//printf("%hhX\n",rx);
		
		if (memcmp(beacon,receivedBeacon,8)==0)
		{
			printf("I GOT A BEACON\n");
			rxl	= read(uart0_filestream, (void*)&jpegSize, sizeof(uint16_t));
			printf("Size: %d\n", jpegSize);
			uint16_t i = 0;
			while (i < jpegSize) {
				rxl = read(uart0_filestream, jpegBuffer + i, jpegSize - i);
				//syslog(LOG_INFO, "Input: Read %d/%lu bytes", rc, jpg_size-i);
				i += rxl;
			}
			//rxl = read(uart0_filestream, (void*)jpegBuffer, jpegSize);
			printf("Read %d bytes\n",i);
			if (jpegSize>0) {
				readJPG(jpegSize, jpegBuffer);
				//printf("JPEG is read\n");
				ILI9341_Draw_Image((const char*)rgb565_buffer,SCREEN_HORIZONTAL_2);
			}
		}
		/*printf("Reading JPG...");
		readJPG("/home/pi/ILI9341/1.jpg");
		printf("Done\n");*/
		//HAL_Delay(500);
		/*clock_t begin = clock();
		//printf("Capturing...");
		system("/home/pi/ILI9341/cam/do_caputure.sh");
		//printf("Done\n");
		clock_t end = clock();
		double timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("(in %f ms)\n\n",timespent);
		HAL_Delay(130);*/
		
		
		/*printf("Drawing image...");
		ILI9341_Draw_Image((const char*)rgb565_buffer,SCREEN_HORIZONTAL_2);
		printf("Done\n");*/
		//HAL_Delay(500);
		/*begin = clock();
		jpeg = fopen("/home/pi/ILI9341/1.jpg","rb");
		fseek(jpeg, 0, SEEK_END);          // Jump to the end of the file
		int filelen = ftell(jpeg);             // Get the current byte offset in the file		
		rewind(jpeg);                      // Jump back to the beginning of the file

		//printf("Filelen: %d\n",filelen);
		
		buffer = (char *)malloc((filelen+1)*sizeof(char)); // Enough memory for file + \0
		fread(buffer, filelen, 1, jpeg); // Read in the entire file
		fclose(jpeg); // Close the file
		end = clock();
		timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);*/
		//printf("File read (in %f ms)\n\n",timespent);
		//char lol = 'U';
		//fwrite((const void*) &lol,1,1,uart);
		/*
		int count = write(uart0_filestream, &lol, 1);		//Filestream, bytes to write, number of bytes to write
		if (count < 0)
		{
			printf("UART TX error\n");
		}
		*/
		/*begin = clock();
		write(uart0_filestream, (const void*) &beacon,8);
		//HAL_Delay(500);
		write(uart0_filestream, (const void*) &filelen,sizeof(int));
		write(uart0_filestream, buffer,filelen);
		end = clock();
		timespent =(double)(end-begin)/(CLOCKS_PER_SEC/1000);
		//printf("Sent. (in %f ms)\n\n",timespent);
		free(buffer);*/
		
	}
	free(rgb565_buffer);
	//fclose(uart);
}
