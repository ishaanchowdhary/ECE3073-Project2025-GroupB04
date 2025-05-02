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
#define SPI_CONTROLLER_BASE 0x4041000

#define TIME_DISPLAY_BASE 0x4041050

// Gyroscope Addresses
#define GYRO_INT_BASE 0x4041040
#define GYRO_INT_IRQ 3
#define GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID 0

// Keys Addresses
#define KEY10_BASE 0x40410b0
#define KEY10_IRQ 4
#define KEY10_IRQ_INTERRUPT_CONTROLLER_ID 0

// SPI CHIP SELECTS
#define CS_ACCEL 1
#define CS_CAM 0

// Gyroscope Configuration

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
	TAP_THRES, 0x10,
	TAP_AXES, 0x07,
	LATENT, 0x85,
	DUR, 0x40,
	WINDOW, 0xc0,
	BW_RATE, 0x0a,
	INT_ENABLE, 0x60,
	INT_MAP, 0x20,
	POWER_CONTROL, 0x08
};

// Interrupt Flags Define
volatile int tap_flag = 0;	// Double Tap Interrupt Flag
volatile int key_flag = 0;	// Key Interrupt Flag

// --------------------------------------------------------------- To remove
// Global variables for key interrupt
volatile int key1Flag = 0; // Flag for key1 interrupt
// ---------------------------------------------------------------

// Function Declarations
void gyro_isr(void * context);
void key_isr(void * context);
void key1_isr(void* context, alt_u32 id); // Current one in use
void initialise_key1_interrupt();
void combineSobelFilter(uint8_t *sobelX, uint8_t* sobelY, uint8_t*resultImg,int imgW, int imgH);
void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx1 ,int flipImgIdx2 ,int flipImgIdx3 ,int flipImgIdx4);
int getPixelVal(int h, int w, int rawAddress, uint8_t* packedImgArr);
int convolve(uint8_t * inputImg, uint8_t * outputImg, float * kernel, int width, int height);
void display_image_from_array_v3(int imgH, int imgW, uint8_t *image_base, uint32_t display_base,int flipImg);
void Run_Time(uint32_t before, uint32_t after);
int gyro_detect_tap(volatile int tap_flag, int *counter);
int display_select(alt_16 yData);
alt_16 gyro_process_data(alt_u8 readX, alt_u8 readY, alt_u8 readZ, alt_16 xData, alt_16 yData, alt_16 zData);

int main(void){
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

	// Global variables for key interrupt
	volatile int key1Flag = 0; // Flag for key1 interrupt
	volatile int mode = 1;     // 0 = single mode, 1 = quad mode

	// Initialise the interrupt before entering into the loop
	initialise_key1_interrupt();

	// Accelerometer Setup
	alt_u8 gyro_data_in, gyro_data_out, regData;
	alt_u8 readX = READ_X_AXIS;
	alt_u8 readY = READ_Y_AXIS;
	alt_u8 readZ = READ_Z_AXIS;
	alt_16 xData, yData, zData;
	alt_u8 isRes = 0xff;

	for (int i = 0; i < CONFIG_LENGTH; i += 2) {
		alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_ACCEL, 2, gyro_config + i, 0, &gyro_data_out, 0);
	}

	void* context = (void *) &isRes;
	IOWR(GYRO_INT_BASE, 3, 0);
	IOWR(GYRO_INT_BASE, 2, 0x1);
	int gyroISR = alt_ic_isr_register(GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID, GYRO_INT_IRQ, gyro_isr, context, 0x0);

	// Initialising Double Tap Counter for updating Display
	int counter = 0;

	// Initial Setup for Single and Quad Display
	int single_mode = 0;
	int selectedDisp = 0;
	int disp1_mode = 0;
	int disp2_mode = 0;
	int disp3_mode = 0;
	int disp4_mode = 0;

	// Key 1 Interrupt Setup
	while(1){
		// Run time
		uint32_t start = IORD(TIME_DISPLAY_BASE, 0);	// Take Reading at the Start

		// Toggle for switching between Single and Quad Display
		if (key1Flag == 0x1) {
            mode = !mode;  // Toggle mode
            key1Flag = 0;  // Reset the flag
			// When change occurs, set counter to whatever it was on the previous display last - ONLY USE WHEN KEY INTERRUPT TRIGGERS AS DESIRED
//			selectedDisp = display_select(yData);
//			if (mode) {
//				if (selectedDisp==0) counter = disp1_mode;
//				else if (selectedDisp==1) counter = disp2_mode;
//				else if (selectedDisp==2) counter = disp3_mode;
//				else if (selectedDisp==3) counter = disp4_mode;
//			} else {
//				counter = single_mode;
//			}
        }

		// Variable For Switches
		//int sw = IORD(SW_BASE, 0);

		if (mode) {

			// Quad Display Mode
			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtrSmall,9600,&rxArrSmall,0); //SPI for SMALL res

			// Flags For Copying Displays
			int flag1 = 0;
			int flag2 = 0;

			// Select Display using Tilt (About y-axis)
            selectedDisp = display_select(yData);

			// Selected Display Mode updated by Double Tap Interrupt
			if (selectedDisp==0) disp1_mode = counter;
			else if (selectedDisp==1) disp2_mode = counter;
			else if (selectedDisp==2) disp3_mode = counter;
			else if (selectedDisp==3) disp4_mode = counter;
			printf("Display Modes: %d, %d, %d, %d", disp1_mode, disp2_mode, disp3_mode, disp4_mode);
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
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img1 = GlobalBlur;
			} else if (disp1_mode == 3) {
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
			if (disp2_mode == 0) {
				img2 = rxArrSmall;
			} else if (disp2_mode == 1) {
				img2 = rxArrSmall;
				flipImgIdx2 = 1;
			} else if (disp2_mode == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img2 = GlobalBlur;
			} else if (disp2_mode == 3) {
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
			if (disp3_mode == 0) {
				img3 = rxArrSmall;
			} else if (disp3_mode == 1) {
				img3 = rxArrSmall;
				flipImgIdx3 = 1;
			} else if (disp3_mode == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img3 = GlobalBlur;
			} else if (disp3_mode == 3) {
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
			if (disp4_mode == 0) {
				img4 = rxArrSmall;
			} else if (disp4_mode == 1) {
				img4 = rxArrSmall;
				flipImgIdx4 = 1;
			} else if (disp4_mode == 2) {
				if (flag1 == 0) {
					int numOutPixel = convolve(&rxArrSmall,&smallImgBuff1,kernel,160,120);
					GlobalBlur = &smallImgBuff1;
					flag1 = 1;
				}
				img4 = GlobalBlur;
			} else if (disp4_mode == 3) {
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

			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0, 1, sendBuffPtrFull, 38400, &rxArr, 0); // SPI full res

			single_mode = counter; // Uses double tap counter to assign the mode (Normal, Flipped, Blurred or Edge)
			int SingleFlipImgIdx = 0;

			if (single_mode == 0) {
				// Displays normally into the quadrant
			} else if (single_mode == 1) {
				// this will set a variable to be called in the function "display_4_images" as the last input "int flipImgIdx"
				SingleFlipImgIdx = 1;
			} else if (single_mode == 2) {
				int numOutPixel = convolve(&rxArr,&rxArr,kernel,320,240);
			} else if (single_mode == 3) {
				convolve(&rxArr,&sobelArrX,sobelX,320,240);
		    	convolve(&rxArr,&sobelArrY,sobelY,320,240);
		    	combineSobelFilter(sobelArrX,sobelArrY,rxArr,320,240);
			}

			display_image_from_array_v3(240, 320, &rxArr, PIXEL_ADDRESS_BASE_val, SingleFlipImgIdx);
		}

		// Detect double tap event and update counter
		tap_flag = gyro_detect_tap(readX, readY, readZ, xData, yData, zData, tap_flag, &counter);

		// Print rotational data and collect rotation around y-axis
		yData = gyro_process_data(readX, readY, readZ, xData, yData, zData);

		// Run time
	    uint32_t end = IORD(TIME_DISPLAY_BASE, 0);	    // Take Reading at the End
	    Run_Time(start, end); // Call Function to Display Benchmarking
}
}

// Interrupt service routine (ISR) for accelerometer interrupt
void gyro_isr(void * context) {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GYRO_INT_BASE, 0); //clear interrupt
	IOWR(GYRO_INT_BASE, 3, 0);
	tap_flag = 1; //set the tap flag when an interrupt is triggered
}

// Interrupt service routine (ISR) for key interrupt
void key_isr(void * context)  {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE, 0);
	IOWR(KEY10_BASE, 3, 0);
	key_flag = 1 - key_flag;
}

void key1_isr(void* context, alt_u32 id) {
    volatile int* edgeCapturePtr = (volatile int*) context;

    // Read the edge capture register
    *edgeCapturePtr = IORD_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE);

	// Check if interrupt occured
	if(*edgeCapturePtr & 0x2){
		key1Flag = 0x1;
}
    // Clear the edge capture register to enable future interrupts
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE, 0);
}


void initialise_key1_interrupt() {
    // Clear any pending interrupts
    IOWR_ALTERA_AVALON_PIO_EDGE_CAP(KEY10_BASE, 0);

    // Enable interrupts for KEY1 (bit 1)
    IOWR_ALTERA_AVALON_PIO_IRQ_MASK(KEY10_BASE, 0x4);

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

void display_image_from_array_v2(int imgH, int imgW, uint8_t *image_base, uint32_t display_base) {
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

int gyro_detect_tap(volatile int tap_flag, int *counter) {
	// Prints result of triggering accelerometer double tap interrupt and adds to counter

	// Print accelerometer double tap result
	if (tap_flag == 1) {
		printf("\n\nDouble tap detected!\n\n");
		tap_flag = 0;
		*counter = *counter + 1;
	}

	// Reset Counter if over 3
	if (*counter >= 4) {
		*counter = 0;
	}

	return tap_flag;
}

int display_select(alt_16 yData) {
    int selectedDisp = 1;
	if (yData >= -99 && yData < -20) selectedDisp = 2;
	else if (yData > 20 && yData <= 60) selectedDisp = 3;
	else if (yData > 60 && yData <= 90) selectedDisp = 4;
    printf("Selected Display: %d\n", selectedDisp);
	return selectedDisp;
}

alt_16 gyro_process_data(alt_u8 readX, alt_u8 readY, alt_u8 readZ, alt_16 xData, alt_16 yData, alt_16 zData) {
	// Prints rotational data from gyroscope and returns Y-axis rotational data

	// Read accelerometer rotation data
	alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_ACCEL, 1, &readX, 2, &xData, 0x0);
	alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_ACCEL, 1, &readY, 2, &yData, 0x0);
	alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_ACCEL, 1, &readZ, 2, &zData, 0x0);

	printf("X-Axis: %d, Y-Axis: %d, Z-Axis: %d\n", xData, yData, zData);

	return yData;
}
