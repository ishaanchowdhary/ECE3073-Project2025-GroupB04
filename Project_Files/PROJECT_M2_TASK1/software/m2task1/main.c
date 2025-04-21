/*
Student IDs: Student IDs: 33115303, 33867860, 33893012, 3311316
*/
#include "io.h"
#include "system.h"
#include "stdio.h"
#include "stdint.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>
#include <altera_avalon_pio_regs.h>

#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x4041080
#define PIXEL_DATA_BASE_val 0x4041070

#define CAM_READY_BASE 0x4041060
//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x04041000

#define TIME_DISPLAY_BASE 0x4041050
#define GYRO_INT_BASE 0x4041040
#define GYRO_INT_IRQ 3
#define GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define HEX20_BASE 0x40410a0
#define HEX53_BASE 0x4041090

// Read values
#define READ_X_AXIS (0xC0 | 0x32)
#define READ_Y_AXIS (0xC0 | 0x34)
#define READ_Z_AXIS (0xC0 | 0x36)
#define INT_SOURCE (0x30 | 0xC0)


//volatile int tap_flag = 1; // tap detected when i make tap_flag 1, but it is only detected once
volatile int tap_flag = 0; // Flag to indicate double tap
//interrupt service routine (ISR) for accelerometer interrupt
void gyro_isr(void* context) {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GYRO_INT_BASE, 0); //clear interrupt
	tap_flag = 1; //set the tap flag when an interrupt is triggered
}

// FUNCTION to initialise the accelerometer for double tap detection
void init_accelerometer() {
	alt_u8 config[] = {
			0x31, 0x0B, // DATA_FORMAT: full res, +-16g
			0x1D, 0x30, // THRESH_TAP: ~3g
			0x21, 0x20,
			0x22, 0x20,
			0x23, 0x40,
			0x2A, 0x07, // TAP_AXES: X, Y, Z
			0x2E, 0x60,
			0x2F, 0x00,
			0x2D, 0x08
	};

	for (int i = 0; i < sizeof(config); i += 2) {
		alt_avalon_spi_command(SPI_CONTROLLER_BASE, 1, 2, &config[i], 0, NULL, 0);
	}
}

// Function to read axis data from accelerometer
int16_t read_axis_data(alt_u8 register_address) {
	alt_u8 readBuff[2];
	alt_avalon_spi_command(SPI_CONTROLLER_BASE, 1, 1, &register_address, 1, readBuff, 2);
	int16_t data = (readBuff[0]  << 8 | readBuff [1]);
	return data;
}
int main(void){

	init_accelerometer();

	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GYRO_INT_BASE, 0);
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(GYRO_INT_BASE, 0x02); // Enable interrupt on INT2
	alt_ic_isr_register(GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID, GYRO_INT_IRQ, gyro_isr, NULL, 0);


	unsigned char image_buffer[IMAGE_SIZE];

	alt_u8 sendBuff = 0x14;
	alt_u8 *sendBuffPtr = &sendBuff;
	uint8_t rxArr[38400];

	int counter = 100;
	int cameraReady = 0;

    uint8_t *sdram_base_ptr = (uint8_t *) SDRAM_BASE_ADDRESS; // Cast base address to pointer
    uint8_t *pixel_buffer_ptr = (uint8_t *) PIXEL_ADDRESS_BASE_val;


    cameraReady = IORD(CAM_READY_BASE,0);


	IOWR(PIXEL_ADDRESS_BASE_val,0,1);

	while(1){
		uint32_t start = IORD(TIME_DISPLAY_BASE, 0);	// Take Reading at the Start
	    int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtr,38400,&rxArr,0);
	    display_image_from_array(&rxArr,PIXEL_ADDRESS_BASE_val);
	    uint32_t end = IORD(TIME_DISPLAY_BASE, 0);	    // Take Reading at the End
	    Run_Time(start, end); // Call Function to Display Benchmarking


	    // read accelerometer rotation data
	    int16_t x_axis = read_axis_data(READ_X_AXIS);
	    int16_t y_axis = read_axis_data(READ_Y_AXIS);
	    int16_t z_axis = read_axis_data(READ_Z_AXIS);
	    printf("X-Axis: %d, Y-Axis: %d, Z-Axis: %d\n", x_axis, y_axis, z_axis);

	    if (tap_flag) {
	    	tap_flag = 0;
	    	printf("Double tap detected!\n");


	    } else {
	    	printf("Double tap not detected\n");
	    }



}
}


void generate_checkerboard(uint32_t base_address) {
    // Loop through each pixel
	int pixel_count = 320 * 240;
	int counter = 1;
	int pixelPos = 0;

	uint8_t valuesToDisplay[8] = {0x8, 0xA, 0xB, 0xC, 0xD, 0xE, 0xF, 0x7};
	uint32_t combined = 0;

	for (int i = 0; i<pixel_count/8; i++){

		for(int j = 0; j<8; j++){
			combined |= (valuesToDisplay[j] << (28 - j * 4));
		}

        IOWR(base_address, pixelPos, combined);


        pixelPos = pixelPos + 1; //index the sdram by 1 word

	}

}

void display_image(uint32_t image_base, uint32_t display_base) {
    int pixel_count = 320 * 240;  // Total number of pixels (320x240)
    int readAddressOffset = 0; //probs need to change this for muti-image

    // Loop through each pixel
    for (int i = 0; i < pixel_count ; i = i+8) {
    	int pixelValue = IORD(SDRAM_BASE_ADDRESS,readAddressOffset);

//    	IOWR(display_base, 0, pixelValue);
    	for(int j=  0; j<8; j++){


    		uint8_t unpackedValue = (pixelValue >> (28 - j * 4)) & 0xF;
    		IOWR(PIXEL_ADDRESS_BASE_val, 0 , i+j);

    		//specific the value to be displayed
    		IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue);

    	}

    	//specific address of the pixel


    	readAddressOffset = readAddressOffset + 1;


    }
}

void display_image_from_array(uint32_t image_base, uint32_t display_base) {
    int pixel_count = 320 * 240;  // Total number of pixels (320x240)
    int readAddressOffset = 0; //probs need to change this for muti-image

    // Loop through each pixel
    for (int i = 0; i < pixel_count ; i = i+8) {
    	int pixelValue = IORD(image_base,readAddressOffset);
    	uint32_t mask = 0x0000000F;


    	for(int j=  0; j<8; j = j+2){
    		int offesetVal = j+1;

//    		uint8_t unpackedValue = (pixelValue >> (28 - j * 4)) & 0xF;
    		uint8_t unpackedValue = (pixelValue & mask)>>4*j;
    		mask = mask << 4;
    		uint8_t unpackedValue2 = (pixelValue & mask)>>4*(j+1);
    		IOWR(PIXEL_ADDRESS_BASE_val, 0 , i+j);
    		//specific the value to be displayed
    		IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue2);






    		IOWR(PIXEL_ADDRESS_BASE_val, 0 , i+offesetVal);
    		IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue);

    		mask = mask << 4;


    	}


    	//specific address of the pixel

    	readAddressOffset = readAddressOffset + 1;


    }

}

void Run_Time(uint32_t before, uint32_t after){

	// this function will take in the difference after - before for the timer
	// prints the run time, calculates FPS to 2 decimal places and prints FPS
	// separates the 4 digits in FPS to ten, one, tenth, hundredth
	// hex decoder to write a 7 digit binary value to the hex pio's
	// make sure the 8th bit is set to 1 except the HEX_LOW[23:16] where HEX_LOW[23] is set to 0.

	float frameTime = after - before;  // calculates run time of the frame
	printf("The Run Time for the Frame is %.2f\n", frameTime);

	float fps = 1000000.0/frameTime;   // converts us to fps
	printf("The fps for the System is %.2f\n", fps);

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
	IOWR(HEX53_BASE, 0, second_hex_value);

}
