#include "sys/alt_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "io.h"
#include "system.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_mutex.h"
#include "altera_avalon_pio_regs.h"
#include <stdio.h>

//int *sharedMsgBuff = (int*)0x03200000;

int *display1Ready = (int*)0x03600010;
int *display2Ready = (int*)0x03500020;

int *bufferFlag1 = (int*)0x03200030; //use as buffer idx
int *frame1Ready = (int*)0x03200040;


int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;

int *frame2Ready = (int*)0x03200090;

//int *sharedMsgBuff = (int*)0x03ff0000;
#define SHARED_BUFF_1_BASE  0x03300100
#define SHARED_BUFF_2_BASE  0x033096ff

#define SHARED_BUFF_3_BASE	0x03310000
#define SHARED_BUFF_4_BASE	0x03320000

#define CONV_RESULT_BASE_1 0x03400000
#define CONV_RESULT_BASE_2 0x03410000


//#define SHARED_BUFF_3_BASE  0x03100000
//#define testBuff  0x00030000

void *context;
alt_mutex_dev* mutex;
int received = 0;
int valueFromP1 = 0;

void key_isr(void *isr_context, alt_u32 id){
	//should be triggered when P1 send signal to us

	//reading from shared buffer
//    altera_avalon_mutex_lock(mutex, 1);
	alt_dcache_flush_all();  // After writing
    received = *sharedMsgBuff;
//    altera_avalon_mutex_unlock(mutex);
//
    printf("Received (unsigned): %d\n", received);
//    alt_printf("Received: %x\n", received);
//    alt_printf("Received: %u\n", received);

	 IOWR(INPUT_PROC0_BASE,3,0x1);
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



void display_image_from_array_v2(int imgH, int imgW, uint8_t *image_base) {
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


	    	IOWR(PIXEL_ADDRESS_BASE, 0 , pixelAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE, 0, unpackedValue1);


	    	IOWR(PIXEL_ADDRESS_BASE, 0 , pixelAddress+1);
	    	IOWR(PIXEL_DATA_BASE, 0, unpackedValue2);



		}
	}
	altera_avalon_mutex_unlock(mutex);
	alt_dcache_flush_all();  // After writing

}

void display_4_images(uint8_t *img1, uint8_t *img2, uint8_t *img3, uint8_t *img4,uint32_t display_base ,int flipImgIdx){
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

			if(flipImgIdx == 1){
				inputPixelAddress = w+(inputImgW* (intputImgH-h-1))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*h);
			}

			pixelValue = getPixelVal(0,0,inputPixelAddress,img1);

	    	IOWR(PIXEL_ADDRESS_BASE, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE, 0, pixelValue);
		}
	}
	// image 2
	for(int h=0; h<120; h++){
		for(int w=159; w<319; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx == 2){
				inputPixelAddress = w+(inputImgW* (intputImgH-h-2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*h);
			}

			pixelValue = getPixelVal(0,0,inputPixelAddress,img2);

	    	IOWR(PIXEL_ADDRESS_BASE, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE, 0, pixelValue);
		}
	}
	// image 3
	for(int h=120; h<240; h++){
		for(int w=0; w<159; w++){
			int displayAddress  = w+(displayW*h)-1;

			if(flipImgIdx == 3){
				inputPixelAddress = w+(inputImgW*(intputImgH-(h-intputImgH) -2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*(h-intputImgH));
			}

			pixelValue = getPixelVal(0,0,inputPixelAddress,img3);

	    	IOWR(PIXEL_ADDRESS_BASE, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE, 0, pixelValue);
		}
	}
	//image 4
	for(int h=121; h<240; h++){
		for(int w=159; w<319; w++){
			int displayAddress  = w+(displayW*h)-1;
			if(flipImgIdx == 4){
				inputPixelAddress = w+(inputImgW*(intputImgH-(h-intputImgH)-2))-1;

			}else{
				inputPixelAddress = inputImgW-4-w+(inputImgW*(h-intputImgH));
			}
			pixelValue = getPixelVal(0,0,inputPixelAddress,img4);

	    	IOWR(PIXEL_ADDRESS_BASE, 0 , displayAddress);
	    	//specific the value to be displayed
	    	IOWR(PIXEL_DATA_BASE, 0, pixelValue);
		}
	}





}

void convolve_v2(uint8_t * inputImg, uint8_t * outputImg, float h_kernel[3],float v_kernel[3], int width, int height){
    uint16_t x, y, k;
    float temp_buffer[height][width];
    uint8_t convResiltBuff = 0;
    int outputPixelIdx = 0;
    int KERNEL_SIZE = 3;
    int pixelCount = 0;

//    float h_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};
//    float v_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};

    // Horizontal pass
    for (y = 0; y < height; y++) {
        for (x = 0; x <= width - 1; x++) {
            float sum = 0;
            for (k = 0; k < KERNEL_SIZE; k++) {
                int16_t xi = x + k - 1; // offset: -1, 0, 1
                if (xi >= 0 && xi < width) {
                	int pixelAddress = y*width + xi;
                    sum += h_kernel[k] * getPixelVal(0,0,pixelAddress, inputImg);;
                }
            }
            temp_buffer[y][x] = sum;
        }
    }

    // Vertical pass
    for (y = 0; y <= height - 1; y++) {
        for (x = 0; x <= width - 1; x++) {
            float sum = 0;
            for (k = 0; k < KERNEL_SIZE; k++) {
                int16_t yi = y + k - 1; // offset: -1, 0, 1
                if (yi >= 0 && yi < height) {
                    sum += v_kernel[k] * temp_buffer[yi][x];
                }
            }

            // Clamp to 4-bit range and store
            uint8_t result = (uint8_t)(sum + 0.5f); // round to nearest
            if (result > 15) result = 15; // max 4-bit


            pixelCount = pixelCount + 1;

			if(pixelCount == 2){//once we have 2 pixel, we can store them to the output array
				uint8_t finalResult = (((uint8_t)result)|0xF0) & convResiltBuff; //packing the value together
				outputImg[outputPixelIdx] = finalResult;

				outputPixelIdx = outputPixelIdx + 1;
				pixelCount = 0;
			}else{
				convResiltBuff = ((uint8_t)result<<4) | 0x0F; //padding the bottom 4 bits with 0
			}
        }
    }
}

void combineSobelFilter(uint8_t *sobelX, uint8_t* sobelY, uint8_t*resultImg,int imgW, int imgH){

	int numPixel = imgH*imgW;
	int threshold = 13;
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




int getPixelVal(int h, int w, int rawAddress, uint8_t* packedImgArr){
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


void Run_Time_2_frame(uint32_t before, uint32_t after){

	// this function will take in the difference after - before for the timer
	// prints the run time, calculates FPS to 2 decimal places and prints FPS
	// separates the 4 digits in FPS to ten, one, tenth, hundredth
	// hex decoder to write a 7 digit binary value to the hex pio's
	// make sure the 8th bit is set to 1 except the HEX_LOW[23:16] where HEX_LOW[23] is set to 0.

	float frameTime = (after - before);  // calculates run time of the frame
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

int main()
{

	float  kernel[9] = {
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f,
		    1.0f/9.0f, 1.0f/9.0f, 1.0f/9.0f
		};

    float h_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};
    float v_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};

    float sobelA_h[3] = {-1,0,1};
    float sobelA_v[3] = {1,2,1};

    float sobelB_h[3] = {1,2,1};
    float sobelB_v[3] = {-1,0,1};

	uint8_t sobelArrX[38400];
	uint8_t sobelArrY[38400];
	uint8_t sobelArrCombined[38400];

	alt_putstr("Hello from Nios II!\n");
	IOWR(INPUT_PROC0_BASE,3,0x1);
	IOWR(INPUT_PROC0_BASE,2,0x1);
	alt_irq_register(INPUT_PROC0_IRQ, &context, key_isr);
	int counter  = 0;


	received = 0;

	*display1Ready  = 1;
	uint32_t readBuffer = SHARED_BUFF_1_BASE;
	uint32_t start = IORD(TIMER_0_BASE, 0);

	uint32_t img3 = 0x0;
	uint32_t img4 = 0x0;

	int blurFilter = 1;


	alt_dcache_flush_all();

  while (1){
//	  	  printf("P0 runing\n");
	  	  if(*frame1Ready == 1){
	  		  if(*bufferFlag1 == 0){
	  			  readBuffer = SHARED_BUFF_2_BASE;

	  		  }else if(*bufferFlag1 == 1){
	  			  readBuffer = SHARED_BUFF_1_BASE;

	  		  }
	  		  *display1Ready  = 0;
	  		  alt_dcache_flush_all();

	  		  if(*quadImgMode != 0){

	  			if(*needBlur == 1){
	  				convolve_v2(SHARED_BUFF_1_BASE,CONV_RESULT_BASE_1,h_kernel,v_kernel,160,120);
	  				img3 = CONV_RESULT_BASE_1;
	  			}else{img3 = readBuffer;}

	  			if(*needEdgeDetect != 0){
					convolve_v2(SHARED_BUFF_1_BASE,sobelArrX,sobelA_h,sobelA_v,160,120);
					convolve_v2(SHARED_BUFF_1_BASE,sobelArrY,sobelB_h,sobelB_v,160,120);
					combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,160,120);


					img4 = sobelArrCombined;
	  			}else{img4 = readBuffer;}


	  			display_4_images(readBuffer,readBuffer,img3,img4,PIXEL_ADDRESS_BASE,2);
		  		  *display1Ready  = 1;
		  		  *frame1Ready = 0;
		  		  alt_dcache_flush_all();
	  		  }else{
		  			if(*needBlur == 1){
//		  				convolve_v2(readBuffer,CONV_RESULT_BASE_1,kernel,320,240);
		  				convolve_v2(readBuffer,CONV_RESULT_BASE_1,h_kernel,v_kernel,320,240);
		  				readBuffer = CONV_RESULT_BASE_1;
		  			}
		  			if(*needEdgeDetect != 0 ){
						convolve_v2(SHARED_BUFF_1_BASE,sobelArrX,sobelA_h,sobelA_v,320,240);
						convolve_v2(SHARED_BUFF_1_BASE,sobelArrY,sobelB_h,sobelB_v,320,240);
						combineSobelFilter(sobelArrX,sobelArrY,sobelArrCombined,320,240);

						readBuffer = sobelArrCombined;


		  			}
	  			display_image_from_array_v2(240,320,readBuffer);
		  		  *display1Ready  = 1;
		  		  *frame1Ready = 0;
		  		  alt_dcache_flush_all();
	  		  }


	  	  }

	  	  alt_dcache_flush_all();



	    }

	    return 0;
	  }
