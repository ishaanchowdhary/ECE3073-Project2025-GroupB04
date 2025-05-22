/* 
 * "Small Hello World" example. 
 * 
 * This example prints 'Hello from Nios II' to the STDOUT stream. It runs on
 * the Nios II 'standard', 'full_featured', 'fast', and 'low_cost' example 
 * designs. It requires a STDOUT  device in your system's hardware. 
 *
 * The purpose of this example is to demonstrate the smallest possible Hello 
 * World application, using the Nios II HAL library.  The memory footprint
 * of this hosted application is ~332 bytes by default using the standard 
 * reference design.  For a more fully featured Hello World application
 * example, see the example titled "Hello World".
 *
 * The memory footprint of this example has been reduced by making the
 * following changes to the normal "Hello World" example.
 * Check in the Nios II Software Developers Manual for a more complete 
 * description.
 * 
 * In the SW Application project (small_hello_world):
 *
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 * In System Library project (small_hello_world_syslib):
 *  - In the C/C++ Build page
 * 
 *    - Set the Optimization Level to -Os
 * 
 *    - Define the preprocessor option ALT_NO_INSTRUCTION_EMULATION 
 *      This removes software exception handling, which means that you cannot 
 *      run code compiled for Nios II cpu with a hardware multiplier on a core 
 *      without a the multiply unit. Check the Nios II Software Developers 
 *      Manual for more details.
 *
 *  - In the System Library page:
 *    - Set Periodic system timer and Timestamp timer to none
 *      This prevents the automatic inclusion of the timer driver.
 *
 *    - Set Max file descriptors to 4
 *      This reduces the size of the file handle pool.
 *
 *    - Check Main function does not exit
 *    - Uncheck Clean exit (flush buffers)
 *      This removes the unneeded call to exit when main returns, since it
 *      won't.
 *
 *    - Check Don't use C++
 *      This builds without the C++ support code.
 *
 *    - Check Small C library
 *      This uses a reduced functionality C library, which lacks  
 *      support for buffering, file IO, floating point and getch(), etc. 
 *      Check the Nios II Software Developers Manual for a complete list.
 *
 *    - Check Reduced device drivers
 *      This uses reduced functionality drivers if they're available. For the
 *      standard design this means you get polled UART and JTAG UART drivers,
 *      no support for the LCD driver and you lose the ability to program 
 *      CFI compliant flash devices.
 *
 *    - Check Access device drivers directly
 *      This bypasses the device file system to access device drivers directly.
 *      This eliminates the space required for the device file system services.
 *      It also provides a HAL version of libc services that access the drivers
 *      directly, further reducing space. Only a limited number of libc
 *      functions are available in this configuration.
 *
 *    - Use ALT versions of stdio routines:
 *
 *           Function                  Description
 *        ===============  =====================================
 *        alt_printf       Only supports %s, %x, and %c ( < 1 Kbyte)
 *        alt_putstr       Smaller overhead than puts with direct drivers
 *                         Note this function doesn't add a newline.
 *        alt_putchar      Smaller overhead than putchar with direct drivers
 *        alt_getchar      Smaller overhead than getchar with direct drivers
 *
 */

#include "sys/alt_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"

#include "system.h"
#include "io.h"
#include "sys/alt_irq.h"
#include "altera_avalon_mutex.h"
#include "altera_avalon_spi_regs.h"
#include "altera_avalon_pio_regs.h"
#include <stdio.h>


int *display1Ready = (int*)0x03600010;

int *bufferFlag1 = (int*)0x03200020; //use as buffer idx
int *frame1Ready = (int*)0x03200030;


int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;

//int *sharedMsgBuff = (int*)0x03ff0000;
#define SHARED_BUFF_1_BASE  0x03300100
#define SHARED_BUFF_2_BASE  0x033096ff

#define CONV_RESULT_BASE_1 0x03400000
#define CONV_RESULT_BASE_2 0x03410000

// GYRO ADDRESS
#define GYRO_INT_BASE 0x4001020
#define GYRO_INT_IRQ 3
#define GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID 0
// SPI CHIP SELECT
#define CS_ACCEL 1

// GYRO CONFIGURATION
// Gyroscope Write Registers
#define BW_RATE 0x2C
#define POWER_CONTROL 0x2d
#define DATA_FORMAT 0x31
#define INT_ENABLE 0x2E
#define INT_MAP 0x2F
#define THRESH_ACT 0x24
#define THRESH_INACT 0x25
#define TIME_INACT 0x26
#define ACT_INACT_CTL 0x27
#define THRESH_FF 0x28
#define TIME_FF 0x29
#define TAP_AXES 0x2a
#define TAP_THRES 0x1d
#define DUR 0x21
#define LATENT 0x22
#define WINDOW 0x23
// Gyroscope Read Registers
#define INT_SOURCE 0x30
#define X_LB 0x32
#define X_HB 0x33
#define Y_LB 0x34
#define Y_HB 0x35
#define Z_LB 0x36
#define Z_HB 0x37
#define CONFIG_LENGTH 16 * 2
#define MAX_COUNT 500000
// Gyroscope Read Axis Values
#define READ_X_AXIS (0xc0 | X_LB)
#define READ_Y_AXIS (0xc0 | Y_LB)
#define READ_Z_AXIS (0xc0 | Z_LB)

// Functionality to initialise the accelerometer for double tap detection
alt_u8 gyro_config[CONFIG_LENGTH] = {
	DATA_FORMAT, 0x0b,		// 4-wire SPI, full resolution, +/- 16g
	THRESH_ACT, 0x04,
	THRESH_INACT, 0x02,
	TIME_INACT, 0x02,
	ACT_INACT_CTL, 0xff,
	THRESH_FF, 0x09,
	TIME_FF, 0x46,
	TAP_THRES, 0x20,
	TAP_AXES, 0x07,
	LATENT, 0x85,
	DUR, 0x40,
	WINDOW, 0xc0,
	BW_RATE, 0x0a,
	INT_ENABLE, 0x60,
	INT_MAP, 0x20,
	POWER_CONTROL, 0x08
};

//volatile uint8_t *rxArr = (uint8_t *)SHARED_BUFF_1_BASE;
volatile int tap_flag = 0; // Global Variable for double tap interrupt.
void *context;
alt_u8 isRes = 0xff;
void* gyro_context = (void *) &isRes;
alt_mutex_dev* mutex;
int received = 0;
int valueFromP0 = 0;
//uint8_t rxArr[38400];

void KEY_ISR(void *isr_context, alt_u32 id){

//	valueFromP0 = IORD(P1_IN_BASE,0); // message from PIO
//
////	//reading from shared buffer
//    altera_avalon_mutex_lock(mutex, 1);
	alt_dcache_flush_all();  // After writing
    received = *sharedMsgBuff;
//    altera_avalon_mutex_unlock(mutex);
//
    printf("Received (unsigned): %d\n", received);
    alt_printf("Received: %x\n", received);
    alt_printf("Received: %u\n", received);

	 IOWR(P1_IN_BASE,3,0x1);
}

// GYRO INTERRUPT SERVICE
void GYRO_ISR(void * context) {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GYRO_INT_BASE, 0);
	IOWR(GYRO_INT_BASE, 3, 0);
	tap_flag = 1;
}
void send_msg(int msg){

//	altera_avalon_mutex_lock(mutex,1);
	*sharedMsgBuff = msg;
	alt_dcache_flush_all();  // After writing
//	altera_avalon_mutex_unlock(mutex);

	//pulsing the output
	IOWR(P1_OUT_BASE,0,1);
	IOWR(P1_OUT_BASE,0,0);

	printf("P1: sending msg %d to P0 \n",msg);
}

int display_image_from_array_v2(int imgH, int imgW, uint8_t *image_base) {
	//FASTER FUNCTION FOR DISPLAY, DO NOT USE TILL M3
//	int pixel_count = imgH*imgW;
//	int readAddressOffset = 0;
	int pixel_idx = 0;
//	int imgH = 238;
//	int imgW = 318;

//	int pixelAddress_2 = 0;

//	Address = Column + (Row * Width)
	altera_avalon_mutex_lock(mutex,1);
	for(int h = 0; h<imgH; h=h+1){
		for(int w = imgW-1; w>=0; w=w-2){

			int pixelAddress  = w+(imgW*h)-4;
			int pixelValue = image_base[pixel_idx];
			pixel_idx = pixel_idx + 1;

			int mask = 0x0F;

			int unpackedValue1 = (pixelValue&mask);
			mask = mask << 4;
			int unpackedValue2 = (pixelValue&mask)>>4;

//			0x4001060
	    	IOWR(0x4001060, 0 , pixelAddress);
	    	//specific the value to be displayed
	    	IOWR(0x4001050, 0, unpackedValue1);


	    	IOWR(0x4001060, 0 , pixelAddress+1);
	    	IOWR(0x4001050, 0, unpackedValue2);



		}
	}
	altera_avalon_mutex_unlock(mutex);
	alt_dcache_flush_all();  // After writing
	return 1;

}

void Run_Time_2_frame(uint32_t before, uint32_t after){

	// this function will take in the difference after - before for the timer
	// prints the run time, calculates FPS to 2 decimal places and prints FPS
	// separates the 4 digits in FPS to ten, one, tenth, hundredth
	// hex decoder to write a 7 digit binary value to the hex pio's
	// make sure the 8th bit is set to 1 except the HEX_LOW[23:16] where HEX_LOW[23] is set to 0.

	float frameTime = (after - before);  // calculates run time of the frame
//	printf("The Run Time for the Frame is %.2f\n", frameTime);

	float fps = 1000000.0/frameTime;   // converts us to fps
//	fps = fps*2;
//	printf("The fps for the System is %.2f\n", fps);

	uint16_t fps_x100 = fps*100; // Multiply the fps by 100 to retain the 2 decimal places

	// Break into digits: ab.cd --> 1000a + 100b + 10c + d
	uint8_t d0 = fps_x100 % 10;
	uint8_t d1 = (fps_x100 / 10) % 10;
	uint8_t d2 = (fps_x100 / 100) % 10;
	uint8_t d3 = (fps_x100 / 1000) % 10;

	// look up Table for Hex Decoder Values

	uint8_t hex_lut[10] = {
		~0b00111111, // 0
		~0b00000110, // 1
		~0b01011011, // 2
		~0b01001111, // 3
		~0b01100110, // 4
		~0b01101101, // 5
		~0b01111101, // 6
		~0b00000111, // 7
		~0b01111111, // 8
		~0b01101111  // 9
	};

	// Convert digits to 7-segment encoding
	uint8_t seg0 = hex_lut[d0] | 0b10000000;  // rightmost digit
	uint8_t seg1 = hex_lut[d1] | 0b10000000;
	uint8_t seg2 = hex_lut[d2] | 0b10000000;
	seg2 &= ~0b10000000;  // Set the final bit in seg2 to 0
	uint8_t seg3 = hex_lut[d3] | 0b10000000;

	// Pack HEX0-HEX2 into first_hex_export (24-bit)
	uint32_t first_hex_value = (seg2 << 16) | (seg1 << 8) | seg0;

	// HEX3 goes into second_hex_export (just the lowest byte)
	uint32_t second_hex_value = seg3;

	// Write to PIOs
	IOWR(HEX20_BASE, 0, first_hex_value);
	IOWR(HEX52_BASE, 0, second_hex_value);

}

int main()
{ 
	//setting up interrupt
	// key_isr
	IOWR(P1_IN_BASE, 3, 0x1);
	IOWR(P1_IN_BASE, 2, 0x1);
	alt_irq_register(P1_IN_IRQ, &context, KEY_ISR);

	// gyro double tap
	IOWR(GYRO_INT_BASE, 3, 0);
	IOWR(GYRO_INT_BASE, 2, 0x1);
	int gyroISR = alt_ic_isr_register(GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID, GYRO_INT_IRQ, GYRO_ISR, gyro_context, 0x0 );


	// Accelerometer Set up
	alt_u8 gyro_data_in, gyro_data_out, regData;

	for (int i = 0; i < CONFIG_LENGTH; i += 2) {
		alt_avalon_spi_command(SPI_0_BASE, CS_ACCEL, 2, gyro_config + i, 0, &gyro_data_out, 0);
	}



	// setting up value for SPI
	alt_u8 sendBuffFull = 0x14; //send buffer for packed data, full res

	alt_u8 *sendBuffPtrFull = &sendBuffFull;

	alt_u8 sendBuffSmall = 0x16;
	alt_u8 *sendBuffPtrSmall = &sendBuffSmall;

  /* Event loop never exits. */
	int counter  = 0;
	received = 0;
	*bufferFlag1 = 1;




	uint32_t writeBuffer1 = SHARED_BUFF_1_BASE;

	alt_avalon_spi_command(SPI_0_BASE, 0 ,1,sendBuffPtrFull,38400,writeBuffer1,0); //SPI for full res
//	alt_avalon_spi_command(SPI_0_BASE, 0 ,1,sendBuffPtrFull,38400,writeBuffer2,0); //SPI for full res
	//image processing setting
	*needBlur = 1;
	*display1Ready = 0;

	*quadImgMode = 0;
	*needEdgeDetect = 0;



	alt_dcache_flush_all();  // After writing
	uint32_t start = IORD(TIME_DISPLAY_BASE, 0);

	  while (1){
//		  printf("P1 running display ready is %d\n", *display1Ready);


		  if(*display1Ready == 1){
			  if(*bufferFlag1 == 0){
				  writeBuffer1 = SHARED_BUFF_1_BASE;
			  }else if(*bufferFlag1 == 1){
				  writeBuffer1 = SHARED_BUFF_2_BASE;
			  }
			  *frame1Ready = 1;
			  *display1Ready = 0;

			  alt_dcache_flush_all();  // After writing

			  if(*quadImgMode != 0){
				  alt_avalon_spi_command(SPI_0_BASE, 0 ,1,sendBuffPtrSmall,9600,writeBuffer1,0); //SPI for full res
			  }else{
				  alt_avalon_spi_command(SPI_0_BASE, 0 ,1,sendBuffPtrFull,38400,writeBuffer1,0); //SPI for full res
				  usleep(29000);
			  }
			  *bufferFlag1 = !*bufferFlag1;
			  uint32_t end = IORD(TIME_DISPLAY_BASE, 0);
	  		  Run_Time_2_frame(start, end);
	  		  start = IORD(TIME_DISPLAY_BASE, 0);

	  		  // read gyro input
	  		gyro_data_in = INT_SOURCE | 0x80;
	  		alt_avalon_spi_command(SPI_0_BASE, CS_ACCEL, 1, &gyro_data_in, 1, &regData, 0x0);
		  }


//		 *needBlur  = IORD(0x040010a0,0)&0x1;
		 *quadImgMode  = IORD(0x040010a0,0)&0x2; // CONFUSED why is quadImgMode being switched with a double tap shouldn't that be a key interrupt
//		 *needEdgeDetect = IORD(0x040010a0,0)&0x4;


		 // HANDLE DOUBLE TAP TOGGLE
		 if (tap_flag == 1) {
			 printf("\n\nDouble Tap Detected !\n\n");
			 tap_flag = 0;
			 counter++;

			 if (counter >= 3) counter = 0;

			 // reset all modes
			 *needBlur = 0;
			 *needEdgeDetect = 0;

			 switch (counter) {
			   case 0:
				   printf("Mode 0: All effects OFF\n");
				   break;
			   case 1:
				   *needBlur = 1;
				   printf("Mode 1: Blur enabled\n");
				   break;
			   case 2:
				   *needEdgeDetect = 1;
				   printf("Mode 3: Edge Detection enabled\n");
				   break;




			 }

			 alt_dcache_flush_all();

		 }
//		  printf("needEdgeDetect %d \n",*needEdgeDetect);
//		  printf("quadImgMode is is %d \n",*quadImgMode);
		  alt_dcache_flush_all();



	  }
	  return 0;
	}
