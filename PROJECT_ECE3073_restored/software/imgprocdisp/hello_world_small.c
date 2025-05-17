// Display Processor

#include "sys/alt_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "io.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_mutex.h"
#include "altera_avalon_pio_regs.h"


// ------------- TODO: Remove address values and change to match in code ---------------------------
#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x4041080
#define PIXEL_DATA_BASE_val 0x4041070

#define CAM_READY_BASE 0x4041060
//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x4041000

#define TIME_DISPLAY_BASE 0x4041050

// Keys Addresses
#define KEY10_BASE 0x40410b0
#define KEY10_IRQ 4
#define KEY10_IRQ_INTERRUPT_CONTROLLER_ID 0

//input and output proc 0
#define INPUT_PROC0_BASE 0x4001030
#define OUTPUT_PROC0_BASE 0x4001020

// HEX
#define HEX20_BASE 0x4001080
#define HEX53_BASE 0x4001070
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Keep these: modify to correct val---------------------------------------------------------------------
int *display1Ready = (int*)0x03600010;
int *display2Ready = (int*)0x03500020;
int *bufferFlag1 = (int*)0x03200030; //use as buffer idx
int *frame1Ready = (int*)0x03200040;


int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;

int *frame2Ready = (int*)0x03200090;

#define SHARED_BUFF_1_BASE  0x03300100
#define SHARED_BUFF_2_BASE  0x033096ff

#define SHARED_BUFF_3_BASE	0x03310000
#define SHARED_BUFF_4_BASE	0x03320000

#define CONV_RESULT_BASE_1 0x03400000
#define CONV_RESULT_BASE_2 0x03410000

void *context;
alt_mutex_dev* mutex;
int received = 0;
int valueFromP1 = 0;
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// Global variables for key interrupt
volatile int Key1Flag = 0; // Flag for key1 interrupt

// Function Declarations
void key1_isr(void* context, alt_u32 id); // Current one in use
void initialise_key1_interrupt();
void combineSobelFilter(uint8_t *sobelX, uint8_t* sobelY, uint8_t*resultImg,int imgW, int imgH);
void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx1 ,int flipImgIdx2 ,int flipImgIdx3 ,int flipImgIdx4);
int getPixelVal(int h, int w, int rawAddress, uint8_t* packedImgArr);
int convolve(uint8_t * inputImg, uint8_t * outputImg, float * kernel, int width, int height);
//void display_image_from_array_v3(int imgH, int imgW, uint8_t *image_base, uint32_t display_base,int flipImg);
void display_image_from_array_v2(int imgH, int imgW, uint8_t *image_base, uint32_t display_base,int flipImg);
void Run_Time(uint32_t before, uint32_t after);
void display_select(int config_mode, int* selectedDisp1, int* selectedDisp2, int* selectedDisp3, int* selectedDisp4);
void send_msg(int msg);

int main() {
	alt_putstr("Image Display Processor Initialised\n");

	alt_u8 sendBuffFull = 0x14; //send buffer for packed data, full res
	alt_u8 sendBuffSmall = 0x16; //send buffer for packed data, small res

	alt_u8 *sendBuffPtrFull = &sendBuffFull;
	alt_u8 *sendBuffPtrSmall = &sendBuffSmall;
	uint8_t rxArr[38400];
	uint8_t rxArrSmall[9600];

	uint8_t smallImgBuff1[9600];

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

	// Variable for key interrupt
	volatile int mode = 1;     // 0 = single mode, 1 = quad mode

	// Initialise the interrupt before entering into the loop
	initialise_key1_interrupt();

	// Initial Setup for Single and Quad Display
	int single_mode = 0;

	int D1;
	int D2;
	int D3;
	int D4;
	int config_mode;

	while(1){
		// Run time
		uint32_t start = IORD(TIME_DISPLAY_BASE, 0);	// Take Reading at the Start

		// Toggle for switching between Single and Quad Display
		if (Key1Flag == 1) {
            mode = !mode;  // Toggle mode
            Key1Flag = 0;  // Reset the flag
        }

		if (mode) {
			// Quad Display Mode
			// Run on other proc: int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtrSmall,9600,&rxArrSmall,0); //SPI for SMALL res
			// Flags For Copying Displays
			int flag1 = 0;
			int flag2 = 0;
			// TODO ---------------------------------Load config mode val from SDRAM
			config_mode = 0;
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			display_select(config_mode, &D1, &D2, &D3, &D4);

			// All the same from here

			// Flip Index
			int flipImgIdx1 = 0;
			int flipImgIdx2 = 0;
			int flipImgIdx3 = 0;
			int flipImgIdx4 = 0;

			// Handle Disp1
			if (D1 == 0) {
				img1 = &rxArrSmall;
			} else if (D1 == 1) {
				img1 = &rxArrSmall;
				flipImgIdx1 = 1;
			} else if (D1 == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img1 = GlobalBlur;
			} else if (D1 == 3) {
				if (flag2 == 0) {
					convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img1 = GlobalEdge;
			}


			// Handle Disp2
			if (D2 == 0) {
				img2 = rxArrSmall;
			} else if (D2 == 1) {
				img2 = rxArrSmall;
				flipImgIdx2 = 1;
			} else if (D2 == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img2 = GlobalBlur;
			} else if (D2 == 3) {
				if (flag2 == 0) {
					convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img2 = GlobalEdge;
			}

			// Handle Disp3
			if (D3 == 0) {
				img3 = rxArrSmall;
			} else if (D3 == 1) {
				img3 = rxArrSmall;
				flipImgIdx3 = 1;
			} else if (D3 == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img3 = GlobalBlur;
			} else if (D3 == 3) {
				if (flag2 == 0) {
					convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img3 = GlobalEdge;
			}

			// Handle Disp4
			if (D4 == 0) {
				img4 = rxArrSmall;
			} else if (D4 == 1) {
				img4 = rxArrSmall;
				flipImgIdx4 = 1;
			} else if (D4 == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img4 = GlobalBlur;
			} else if (D4 == 3) {
				if (flag2 == 0) {
					convolve(&rxArrSmall,&sobelArrX,sobelX,160,120);
					convolve(&rxArrSmall,&sobelArrY,sobelY,160,120);
					combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);
					GlobalEdge = &sobelArrCombined;
					flag2 = 1;
				}
				img4 = GlobalEdge;
			}

			display_4_images(img1, img2, img3, img4, PIXEL_ADDRESS_BASE_val, flipImgIdx1, flipImgIdx2, flipImgIdx3, flipImgIdx4);

		} else {
			// Single Display Mode

			//int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0, 1, sendBuffPtrFull, 38400, &rxArr, 0); // SPI full res
			// Run this line in other processor ^^^^^
			single_mode = 0; // TODO: ------------------------ Connect to counter from double tap on other processor by storing two bits (0-3) in sdram and load here
			int SingleFlipImgIdx = 0;

			// Mode Selection
			if (single_mode == 0) {
				// Default Mode
			} else if (single_mode == 1) {
				// Flipped Mode
				SingleFlipImgIdx = 1; // this will set a variable to be called in the function "display_4_images" as the last input "int flipImgIdx"
			} else if (single_mode == 2) {
				// Blurred Mode
				int numOutPixel = convolve(&rxArr,&rxArr,kernel,320,240);
			} else if (single_mode == 3) {
				// Edge Detection Mode
				convolve(&rxArr,&sobelArrX,sobelX,320,240);
		    	convolve(&rxArr,&sobelArrY,sobelY,320,240);
		    	combineSobelFilter(sobelArrX,sobelArrY,rxArr,320,240);
			}
			printf("Single Image Mode: %d\n", single_mode);
			display_image_from_array_v2(240, 320, &rxArr, PIXEL_ADDRESS_BASE_val, SingleFlipImgIdx);
		}

		// Run time
	    uint32_t end = IORD(TIME_DISPLAY_BASE, 0);	    // Take Reading at the End
	    Run_Time(start, end); // Call Function to Display Benchmarking
	}

	return 0;
}


void key1_isr(void* context1, alt_u32 id) {
    volatile int* edgeCapturePtr = (volatile int*) context1;

    // Read the edge capture register
    *edgeCapturePtr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE);

	// Check if interrupt occured
	if(*edgeCapturePtr & 0x2){
		printf("KEY1 Interrupt Detected.\n");
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

void combineSobelFilter(uint8_t *sobelX, uint8_t* sobelY, uint8_t*resultImg,int imgW, int imgH) {

	int numPixel = imgH*imgW;
	int threshold = 11;
	int pixelIdx = 0;
	uint8_t packedPixelVal = 0;
	for(int i=0; i< numPixel; i = i+1){
		//pixel 1
		int pixelValue1 = ((sobelX[pixelIdx] & 0xF0)>>4) + ((sobelY[pixelIdx] & 0xF0)>>4);//combining x,y value after conv

		if (pixelValue1 < threshold){
			pixelValue1 = 0; //blacking out pixel value if they are smaller than threshold
		}
		//pixel 2
		int pixelValue2 = ((sobelX[pixelIdx] & 0x0F))+((sobelY[pixelIdx] & 0x0F));

		if (pixelValue2 < threshold){
			pixelValue2 = 0;
		}

		packedPixelVal = (((uint8_t)pixelValue2)|0xF0) & (((uint8_t)pixelValue1<<4) | 0x0F); //packing 2 pixel value into 1 byte
		resultImg[i] = packedPixelVal;

		pixelIdx = pixelIdx + 1;
	}
}

void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx1 ,int flipImgIdx2 ,int flipImgIdx3 ,int flipImgIdx4) {
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

	int pixelValue = 0;
	//image 1
	for(int h=0; h<120; h++){
		for(int w=0; w<159; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx1 == 1){
				inputPixelAddress = w+(inputImgW* (intputImgH-h-1))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*h);
			}

			pixelValue = getPixelVal(0,0,inputPixelAddress,img1);

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

			pixelValue = getPixelVal(0,0,inputPixelAddress,img2);

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

			pixelValue = getPixelVal(0,0,inputPixelAddress,img3);

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
			pixelValue = getPixelVal(0,0,inputPixelAddress,img4);

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);
		}
	}
}

int getPixelVal(int h, int w, int rawAddress, uint8_t* packedImgArr) {
	//function to get pixel value from a packed array
	int pixelOutput = 0;
	int packedPixelVal = 0;
	int decodedAddress;

	if (rawAddress % 2 == 0) { //if the value is even we need to extract the top 4 bits

	    decodedAddress = rawAddress/2;
	    packedPixelVal = packedImgArr[decodedAddress];

	    pixelOutput = (packedPixelVal & 0xF0)>>4;
	} else { //if the value is odd we need to extract the bottom 4 bits
	    decodedAddress = (rawAddress-1)/2;
	    packedPixelVal = packedImgArr[decodedAddress];

	    pixelOutput = (packedPixelVal & 0x0F); //extracting the value for a single pixel from the packed storage
	}
	return pixelOutput;
}

int convolve(uint8_t * inputImg, uint8_t * outputImg, float * kernel, int width, int height) {
	int outputPixelIdx = 0;
	int pixelCount = 0;
	uint8_t convResiltBuff = 0;

	for(int h=0; h<=height-1;h = h+1){
		for(int w=0; w<=width-1;w = w + 1){
			float convResult = 0;
			float currentPixelVal = 0;
			float currentKernelVal;

			//looping through each offset pixel locations overlapped by the kernel
			for(int kh = -1; kh<=1; kh++){
				for(int kw = -1; kw<=1; kw++){
					int kernelIdx = (kh+1)*3 +(kw+1);
					int pixelAddress = (h+kh)*width + (w+kw);
					currentKernelVal = kernel[kernelIdx]; //getting the current value of the kernel
					currentPixelVal = getPixelVal(0,0,pixelAddress, inputImg); //getting the overlapping pixel location

					convResult = convResult + (currentPixelVal*currentKernelVal);
				}
			}
			pixelCount = pixelCount + 1;

			if(pixelCount == 2){//once we have 2 pixel, we can store them to the output array
				uint8_t finalResult = (((uint8_t)convResult)|0xF0) & convResiltBuff; //packing the value together
				outputImg[outputPixelIdx] = finalResult;

				outputPixelIdx = outputPixelIdx + 1;
				pixelCount = 0;
			}else{
				convResiltBuff = ((uint8_t)convResult<<4) | 0x0F; //padding the bottom 4 bits with 0
			}
		}
	}

return outputPixelIdx;
}

void display_image_from_array_v3(int imgH, int imgW, uint8_t *image_base, uint32_t display_base,int flipImg) {
	// if flipImg is 1, the displayed img will be flipped
	int pixelValue = 0;
	int pixelAddress = 0;
	for(int h = 0; h<=240; h=h+1){
		for(int w = 0; w<=320; w=w+1){

			if(flipImg){
				pixelAddress = w+(imgW* (imgH-h-1))-1;
			}else{
				pixelAddress = w+(imgW*h);

			}

			int pixelAddressOpp = imgW-w+(imgW*h)-4;

			if(h>imgH | w>imgW){
				pixelValue = 15;
			}else{
				pixelValue = getPixelVal(imgH,imgW,pixelAddressOpp,image_base);
			}

	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , pixelAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, pixelValue);

		}
	}

}

void display_image_from_array_v2(int imgH, int imgW, uint8_t *image_base, uint32_t display_base,int flipImg) {
	//FASTER FUNCTION FOR DISPLAY, DO NOT USE TILL M3
	int pixel_count = imgH*imgW;
	int readAddressOffset = 0;
	int pixel_idx = 0;
//	int imgH = 238;
//	int imgW = 318;

	int pixelAddress_2 = 0;

//	Address = Column + (Row * Width)
	for(int h = 0; h<=imgH-1; h=h+1){
		for(int w = imgW-1; w>=0; w=w-2){

			int pixelAddress  = w+(imgW*h);
			int pixelValue = image_base[pixel_idx];
			pixel_idx = pixel_idx + 1;

			int mask = 0x0F;

			int unpackedValue1 = (pixelValue&mask);
			mask = mask << 4;
			int unpackedValue2 = (pixelValue&mask)>>4;


	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , pixelAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue1);


	    	IOWR(PIXEL_ADDRESS_BASE_val, 0 , pixelAddress+1);
	    	IOWR(PIXEL_DATA_BASE_val, 0, unpackedValue2);
		}
	}
}

void Run_Time(uint32_t before, uint32_t after) {

	// this function will take in the difference after - before for the timer
	// prints the run time, calculates FPS to 2 decimal places and prints FPS
	// separates the 4 digits in FPS to ten, one, tenth, hundredth
	// hex decoder to write a 7 digit binary value to the hex pio's
	// make sure the 8th bit is set to 1 except the HEX_LOW[23:16] where HEX_LOW[23] is set to 0.

	float frameTime = after - before;  // calculates run time of the frame
	// printf("The Run Time for the Frame is %.2f\n", frameTime);

	float fps = 1000000.0/frameTime;   // converts us to fps
	// printf("The fps for the System is %.2f\n", fps);

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

void display_select(int config_mode, int* selectedDisp1, int* selectedDisp2, int* selectedDisp3, int* selectedDisp4) {
    if (config_mode == 0) {
        *selectedDisp1 = 0;
        *selectedDisp2 = 1;
        *selectedDisp3 = 2;
        *selectedDisp4 = 3;
    }
    else if (config_mode == 1) {
        *selectedDisp1 = 3;
        *selectedDisp2 = 0;
        *selectedDisp3 = 1;
        *selectedDisp4 = 2;
    }
    else if (config_mode == 2) {
        *selectedDisp1 = 2;
        *selectedDisp2 = 3;
        *selectedDisp3 = 0;
        *selectedDisp4 = 1;
    }
    else if (config_mode == 3) {
        *selectedDisp1 = 1;
        *selectedDisp2 = 2;
        *selectedDisp3 = 3;
        *selectedDisp4 = 0;
    }
}

void send_msg(int msg){

//	altera_avalon_mutex_lock(mutex,1);
	*sharedMsgBuff = msg;
//	alt_dcache_flush_all();  // After writing

	alt_dcache_flush_all();  // After writing
//	altera_avalon_mutex_unlock(mutex);

	//pulsing the output
	IOWR(OUTPUT_PROC0_BASE,0,1);
	IOWR(OUTPUT_PROC0_BASE,0,0);

	printf("P0: sending %d to P1 \n",msg);
}
