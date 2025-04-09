#include "io.h"
#include "system.h"
#include "stdio.h"
#include "stdint.h"
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>

#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x04041060
#define PIXEL_DATA_BASE_val 0x04041050

#define CAM_READY_BASE 0x04041040
//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x04041000



int main(void){
	unsigned char image_buffer[IMAGE_SIZE];

//	uint32_t PIXEL_ADDRESS_BASE_val = 67375168;
//	uint32_t PIXEL_DATA_BASE_val = 67375152;

	//===========SPI===========//
//	uint8_t sendBuff = 0x41; //8 bits control code to camera
	alt_u8 sendBuff = 0x14;
	alt_u8 *sendBuffPtr = &sendBuff;
	uint8_t rxArr[38400];

//	uint8_t *buffer_ptr = (uint8_t *)SDRAM_BASE_ADDRESS;

	//Base address,
//	CPI CS number,
//	size of send buff,
//	pointer to send buff,
//	size of receive buff,
//	pointer to receive buff,
//	flag


	int counter = 100;
	int cameraReady = 0;

    uint8_t *sdram_base_ptr = (uint8_t *) SDRAM_BASE_ADDRESS; // Cast base address to pointer
    uint8_t *pixel_buffer_ptr = (uint8_t *) PIXEL_ADDRESS_BASE_val;
//    generate_checkerboard(SDRAM_BASE_ADDRESS);  // Pass pointer to function

    cameraReady = IORD(CAM_READY_BASE,0);
    display_image(SDRAM_BASE_ADDRESS, PIXEL_ADDRESS_BASE_val);

//    int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtr,38400,&rxArr,0);

    display_image_from_array(&rxArr,PIXEL_ADDRESS_BASE_val);

//    display_image(SDRAM_BASE_ADDRESS, PIXEL_ADDRESS_BASE_val);

	IOWR(PIXEL_ADDRESS_BASE_val,0,1);

	while(1){
	    int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtr,38400,&rxArr,0);
	    display_image_from_array(&rxArr,PIXEL_ADDRESS_BASE_val);
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

//    	IOWR(display_base, 0, pixelValue);
    	for(int j=  0; j<8; j++){


//    		uint8_t unpackedValue = (pixelValue >> (28 - j * 4)) & 0xF;
    		uint8_t unpackedValue = (pixelValue & mask)>>4*j;
    		IOWR(PIXEL_ADDRESS_BASE_val, 0 , i+j);


    		//specific the value to be displayed

    		IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue);

    		mask = mask << 4;

    	}

    	//specific address of the pixel


    	readAddressOffset = readAddressOffset + 1;


    }
}
