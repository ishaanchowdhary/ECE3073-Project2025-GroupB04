#include "io.h"
#include "system.h"
#include <stdio.h>
#include <stdint.h>
#include <altera_avalon_spi.h>
#include <altera_avalon_spi_regs.h>
#include "altera_avalon_pio_regs.h"
#include "alt_types.h"



#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x4041070
#define PIXEL_DATA_BASE_val 0x4041060

#define CAM_READY_BASE 0x04041050
//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x04041000

#define TIME_DISPLAY_BASE 0x4041040

#define KEY10_BASE 0x40410a0


// Global variables for key interrupt
volatile int Key1Flag = 0; // Flag for key1 interrupt

uint16_t getPixelVal(int rawAddress, uint8_t* packedImgArr);
void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx1 ,int flipImgIdx2 ,int flipImgIdx3 ,int flipImgIdx4);
void Run_Time(uint32_t before, uint32_t after);
int main(void);
void key1_isr(void* context, alt_u32 id);
void initialise_key1_interrupt();
void display_single_image(int imgH, int imgW, const uint8_t *src, int flipImg);




int main(void){
	alt_u8 sendBuffFull = 0x15;  //send buffer for RGB packed data, full res
	alt_u8 sendBuffSmall = 0x17; //send buffer for RGB packed data, small res

	alt_u8 *sendBuffPtrFull = &sendBuffFull;
	alt_u8 *sendBuffPtrSmall = &sendBuffSmall;
	uint8_t rxArr[115200];
	uint8_t rxArrSmall[28800];

	uint8_t smallImgBuff1[28800];

	float  kernel[9] = {
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f
		};

	float  sobelX[9] = {
		    1.0f, 0.0f, -1.0f,
			2.0f, 0.0f, -2.0f,
			1.0f, 0.0f, -1.0f
		};

	float  sobelY[9] = {
		    1.0f, 2.0f, 1.0f,
			0.0f, 0.0f, 0.0f,
			-1.0f, -2.0f, -1.0f
		};

	//Sobel Filter Varibles
	uint8_t sobelArrX[38400];
	uint8_t sobelArrY[38400];
	uint8_t sobelArrCombined[38400];



	// Image Variables
	uint8_t *img1;
	uint8_t *img2;
	uint8_t *img3;
	uint8_t *img4;

	// Global Displays To Be Copied From
	uint8_t *GlobalBlur;
	uint8_t *GlobalEdge;

	// // Global variables for key interrupt
	volatile int mode = 0;     // 0 = single mode, 1 = quad mode

	// // Initialise the interrupt before entering into the loop
	initialise_key1_interrupt();

	while(1){


		if (Key1Flag == 1) {
            mode = !mode;  // Toggle mode
            Key1Flag = 0;  // Reset the flag
        }


		// // Variable For Switches
		int sw = IORD(SW_BASE, 0);

		uint32_t start = IORD(TIME_DISPLAY_BASE, 0);	// Take Reading at the Start



		if (mode) {

			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtrSmall,28800 + 1,&rxArrSmall,0); //SPI for SMALL res

			// Flags For Copying Displays
			int flag1 = 0;
			int flag2 = 0;

			// Quad Display Mode Switches
			int disp1_mode = (sw >> 0) & 0x3; // SW[1:0]
			int disp2_mode = (sw >> 2) & 0x3; // SW[3:2]
			int disp3_mode = (sw >> 4) & 0x3; // SW[5:4]
			int disp4_mode = (sw >> 6) & 0x3; // SW[7:6]

			// Flip Index
			int flipImgIdx1 = 0;
			int flipImgIdx2 = 0;
			int flipImgIdx3 = 0;
			int flipImgIdx4 = 0;

			// Handle Disp1
			if (disp1_mode == 0) {
				img1 = &rxArrSmall;
			} else if (disp1_mode == 1) {
				img1 = &rxArrSmall;
				flipImgIdx1 = 1;
			} else if (disp1_mode == 2) {
				if (flag1 == 0) {
					// int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img1 = GlobalBlur;
			} else if (disp1_mode == 3) {
				if (flag2 == 0) {
					// convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					// convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					// combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img1 = GlobalEdge;
			}


			// Handle Disp2
			if (disp2_mode == 0) {
				img2 = rxArrSmall;
			} else if (disp2_mode == 1) {
				img2 = rxArrSmall;
				flipImgIdx2 = 1;
			} else if (disp2_mode == 2) {
				if (flag1 == 0) {
					// int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img2 = GlobalBlur;
			} else if (disp2_mode == 3) {
				if (flag2 == 0) {
					// convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					// convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					// combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img2 = GlobalEdge;
			}

			// Handle Disp3
			if (disp3_mode == 0) {
				img3 = rxArrSmall;
			} else if (disp3_mode == 1) {
				img3 = rxArrSmall;
				flipImgIdx3 = 1;
			} else if (disp3_mode == 2) {
				if (flag1 == 0) {
					// int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img3 = GlobalBlur;
			} else if (disp3_mode == 3) {
				if (flag2 == 0) {
					// convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					// convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					// combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img3 = GlobalEdge;
			}

			// Handle Disp4
			if (disp4_mode == 0) {
				img4 = rxArrSmall;
			} else if (disp4_mode == 1) {
				img4 = rxArrSmall;
				flipImgIdx4 = 1;
			} else if (disp4_mode == 2) {
				if (flag1 == 0) {
					// int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img4 = GlobalBlur;
			} else if (disp4_mode == 3) {
				if (flag2 == 0) {
					// convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					// convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					// combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img4 = GlobalEdge;
			}

			display_4_images(img1, img2, img3, img4, PIXEL_ADDRESS_BASE_val, flipImgIdx1, flipImgIdx2, flipImgIdx3, flipImgIdx4);

		} 
		else {

			// Single Display Mode

			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0, 1, sendBuffPtrFull, 115200 + 1, rxArr, 0); // SPI full res

			int single_mode = sw & 0x3; // SW[1:0]
			int SingleFlipImgIdx = 0;

			if (single_mode == 0) {
				// Normal
			} else if (single_mode == 1) {
				// flip
				SingleFlipImgIdx = 1;
			} else if (single_mode == 2) {
				// int numOutPixel = convolve(&rxArr,&rxArr,kernel,320,240);
			} else if (single_mode == 3) {
				// convolve(&rxArr,&sobelArrX,sobelX,320,240);
		    	// convolve(&rxArr,&sobelArrY,sobelY,320,240);
		    	// combineSobelFilter(sobelArrX,sobelArrY,rxArr,320,240);
			}

			display_single_image(240, 320, rxArr, SingleFlipImgIdx);
		}

	    uint32_t end = IORD(TIME_DISPLAY_BASE, 0);	    // Take Reading at the End
	    Run_Time(start, end); // Call Function to Display Benchmarking
}
}

void key1_isr(void* context, alt_u32 id) {
	volatile int* edgeCapturePtr = (volatile int*) context;

	// Read the edge capture register
	*edgeCapturePtr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE);

	// Check if interrupt occured
	if(*edgeCapturePtr & 0x2){
		printf("Interrupt\n");
		Key1Flag = 1;
}

	// Clear the edge capture register to enable future interrupts
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE, 0);
}

void initialise_key1_interrupt() {

	// Clear any pending interrupts
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE, 0);

	// Enable interrupts for KEY1 (bit 1)
	IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY10_BASE, 0x2);

	// Register ISR
	alt_ic_isr_register(
		KEY10_IRQ_INTERRUPT_CONTROLLER_ID, 
		KEY10_IRQ, 
		key1_isr, 
		(void*) KEY10_BASE, 
		0
	);

}

// void combineSobelFilter(uint8_t *sobelX, uint8_t* sobelY, uint8_t*resultImg,int imgW, int imgH){

// 	int numPixel = imgH*imgW;
// 	int threshold = 11;
// 	int pixelIdx = 0;
// 	uint8_t packedPixelVal = 0;
// 	for(int i=0; i< numPixel; i = i+1){
// 		//pixel 1
// 		int pixelValue1 = ((sobelX[pixelIdx] & 0xF0)>>4) + ((sobelY[pixelIdx] & 0xF0)>>4);//combining x,y value after conv

// 		if (pixelValue1 < threshold){
// 			pixelValue1 = 0; //blacking out pixel value if they are smaller than threshold
// 		}
// 		//pixel 2
// 		int pixelValue2 = ((sobelX[pixelIdx] & 0x0F))+((sobelY[pixelIdx] & 0x0F));

// 		if (pixelValue2 < threshold){
// 			pixelValue2 = 0;
// 		}

// 		packedPixelVal = (((uint8_t)pixelValue2)|0xF0) & (((uint8_t)pixelValue1<<4) | 0x0F); //packing 2 pixel value into 1 byte
// 		resultImg[i] = packedPixelVal;

// 		pixelIdx = pixelIdx + 1;

// 	}

// }

// int convolve(uint8_t * inputImg, uint8_t * outputImg, float * kernel, int width, int height){
// 	int outputPixelIdx = 0;
// 	int pixelCount = 0;
// 	uint8_t convResiltBuff = 0;

// 	for(int h=0; h<=height-1;h = h+1){
// 		for(int w=0; w<=width-1;w = w + 1){
// 			float convResult = 0;
// 			float currentPixelVal = 0;
// 			float currentKernelVal;

// 			//looping through each offset pixel locations overlapped by the kernel
// 			for(int kh = -1; kh<=1; kh++){
// 				for(int kw = -1; kw<=1; kw++){
// 					int kernelIdx = (kh+1)*3 +(kw+1);
// 					int pixelAddress = (h+kh)*width + (w+kw);
// 					currentKernelVal = kernel[kernelIdx]; //getting the current value of the kernel
// 					currentPixelVal = getPixelVal(0,0,pixelAddress, inputImg); //getting the overlapping pixel location

// 					convResult = convResult + (currentPixelVal*currentKernelVal);
// 				}
// 			}
// 			pixelCount = pixelCount + 1;

// 			if(pixelCount == 2){//once we have 2 pixel, we can store them to the output array
// 				uint8_t finalResult = (((uint8_t)convResult)|0xF0) & convResiltBuff; //packing the value together
// 				outputImg[outputPixelIdx] = finalResult;

// 				outputPixelIdx = outputPixelIdx + 1;
// 				pixelCount = 0;
// 			}else{
// 				convResiltBuff = ((uint8_t)convResult<<4) | 0x0F; //padding the bottom 4 bits with 0
// 			}
// 		}
// 	}

// return outputPixelIdx;
// }

void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx1 ,int flipImgIdx2 ,int flipImgIdx3 ,int flipImgIdx4){
	//function to display 4 smaller image
	//input
	//	img1: pointer to image top left
	//	img2: pointer to image top right
	//	img3: pointer to image bottom left
	//	img4: pointer to image bottom right
	//	display_base: base address for display
	//	flipImgIdx: The image of which that need to be flipped (i.e if index is 1, top left image is flipped)
	int displayW = 320;
	int inputImgW = 160;
	int intputImgH = 120;
	int inputPixelAddress = 0;

	uint16_t pixelValue = 0;
	//image 1
	for(int h=0; h<120; h++){
		for(int w=0; w<159; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx1 == 1){
				inputPixelAddress = w+(inputImgW* (intputImgH-h-1))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*h);
			}

			pixelValue = getPixelVal(inputPixelAddress,img1);

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);
		}
	}
	// image 2
	for(int h=0; h<120; h++){
		for(int w=159; w<319; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx2 == 1){
				inputPixelAddress = w+(inputImgW* (intputImgH-h-2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*h);
			}

			pixelValue = getPixelVal(inputPixelAddress,img2);

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);
		}
	}
	// image 3
	for(int h=120; h<240; h++){
		for(int w=0; w<159; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx3 == 1){
				inputPixelAddress = w+(inputImgW*(intputImgH-(h-intputImgH) -2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*(h-intputImgH));
			}

			pixelValue = getPixelVal(inputPixelAddress,img3);

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);
		}
	}
	//image 4
	for(int h=120; h<240; h++){
		for(int w=159; w<319; w++){
			int displayAddress  = w+(displayW*h)-1;
			if(flipImgIdx4 == 1){
				inputPixelAddress = w+(inputImgW*(intputImgH-(h-intputImgH)-2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*(h-intputImgH));
			}
			pixelValue = getPixelVal(inputPixelAddress,img4);

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);
		}
	}





}

uint16_t getPixelVal(int rawAddress, uint8_t* packedImgArr) {
    packedImgArr = packedImgArr - 1;

    int pairIndex = rawAddress / 2;
    int byte_idx = pairIndex * 3;

    uint8_t byte0 = packedImgArr[byte_idx];
    uint8_t byte1 = packedImgArr[byte_idx + 1];
    uint8_t byte2 = packedImgArr[byte_idx + 2];

    uint8_t r, g, b;

    if (rawAddress % 2 == 0) {
        // Pixel 1 (even index)
		r = byte1 & 0x0F;
        g = (byte2 >> 4) & 0x0F;
        b = byte2 & 0x0F;
       
    } else {
        // Pixel 2 (odd index)
	    r = (byte0 >> 4) & 0x0F;
        g = byte0 & 0x0F;
        b = (byte1 >> 4) & 0x0F;
    }

    // Pack as 12-bit RGB: RRRR GGGG BBBB
    uint16_t pixelVal = (r << 8) | (g << 4) | b;
    return pixelVal;
}


void display_single_image(int imgH, int imgW, const uint8_t *src, int flipImg) {
    int byte_idx = 0;

    src = src - 1;

    for (int h = 0; h < imgH; h++) {
        // Use flipped row index if needed
        int row;
        if (flipImg == 1) {
            row = imgH - 1 - h;
        } else {
            row = h;
        }

        for (int w = 0; w < imgW; w += 2) {
            // Read 3 bytes per 2 pixels
            uint8_t byte0 = src[byte_idx];
            uint8_t byte1 = src[byte_idx + 1];
            uint8_t byte2 = src[byte_idx + 2];

            // Pixel 1 (RGB order)
            uint8_t r1 = (byte0 >> 4) & 0x0F;
            uint8_t g1 = byte0 & 0x0F;
            uint8_t b1 = (byte1 >> 4) & 0x0F;

            // Pixel 2 (RGB order)
            uint8_t r2 = byte1 & 0x0F;
            uint8_t g2 = (byte2 >> 4) & 0x0F;
            uint8_t b2 = byte2 & 0x0F;

            // Format as RGB: RRRR GGGG BBBB 
            uint16_t pixel1_val = (r1 << 8) | (g1 << 4) | b1;
            uint16_t pixel2_val = (r2 << 8) | (g2 << 4) | b2;

            byte_idx += 3;

            int pixelAddress1 = w + (imgW * row);
            int pixelAddress2 = pixelAddress1 + 1;

            IOWR(PIXEL_ADDRESS_BASE_val, 0, pixelAddress1);
            IOWR(PIXEL_DATA_BASE_val, 0, pixel2_val);

            IOWR(PIXEL_ADDRESS_BASE_val, 0, pixelAddress2);
            IOWR(PIXEL_DATA_BASE_val, 0, pixel1_val);
        }
    }
}



void Run_Time(uint32_t before, uint32_t after){

	// this function will take in the difference after - before for the timer
	// prints the run time, calculates FPS to 2 decimal places and prints FPS
	// separates the 4 digits in FPS to ten, one, tenth, hundredth
	// hex decoder to write a 7 digit binary value to the hex pio's
	// make sure the 8th bit is set to 1 except the HEX_LOW[23:16] where HEX_LOW[23] is set to 0.

	float frameTime = after - before;  // calculates run time of the frame
	//printf("The Run Time for the Frame is %.2f\n", frameTime);

	float fps = 1000000.0/frameTime;   // converts us to fps
	//printf("The fps for the System is %.2f\n", fps);

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
