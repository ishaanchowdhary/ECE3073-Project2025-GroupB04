#include "io.h"
#include "system.h"
#include "stdio.h"
#include "stdint.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>

#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x4041070
#define PIXEL_DATA_BASE_val 0x4041060

#define CAM_READY_BASE 0x04041050
//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x04041000

#define TIME_DISPLAY_BASE 0x4041040


int main(void){
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
