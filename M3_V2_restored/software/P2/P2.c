
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

int *display1Ready = (int*)0x03600010;

int *bufferFlag1 = (int*)0x03200020; //use as buffer idx
int *frame1Ready = (int*)0x03200030;


int *convDone = (int*) 0x03200040;
int *convStart = (int*)0x03200050;

int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;



//int *sharedMsgBuff = (int*)0x03ff0000;
#define SHARED_BUFF_1_BASE  0x03300100
#define SHARED_BUFF_2_BASE  0x033096ff

#define CONV_RESULT_BASE_1 0x03400000
#define CONV_RESULT_BASE_2 0x03410000


//#define SHARED_BUFF_3_BASE  0x03100000
//#define testBuff  0x00030000

void *context;
alt_mutex_dev* mutex;
int received = 0;
int valueFromP1 = 0;

int blurImg(uint8_t * inputImg, uint8_t * outputImg, int width, int height, int startHeight, int endHeight,int outputPixelIdx){
//	int outputPixelIdx = 0;
	int pixelCount = 0;
	int convResiltBuff = 0;

	for(int h=startHeight; h<=endHeight-1;h = h+1){
		for(int w=0; w<=width-1;w = w + 1){
			int convResult = 0;
			uint8_t currentPixelVal = 0;

			int pixelAddress1 = ((h-1)*width + (w)) / 2 ;
			int pixelAddress2 = ((h)*width + (w)) / 2;
			int pixelAddress3 = ((h+1)*width + (w)) / 2;

            uint8_t p1 = inputImg[pixelAddress1-1];
            uint8_t p2 = inputImg[pixelAddress1];

            uint8_t p3 = inputImg[pixelAddress2-1];
            uint8_t p4 = inputImg[pixelAddress2];

            uint8_t p5 = inputImg[pixelAddress3-1];
            uint8_t p6 = inputImg[pixelAddress3];

            uint32_t packed1 = ((uint32_t)p1 << 16) | ((uint32_t)p2 << 8) | p3;
            uint32_t packed2 = ((uint32_t)p4 << 16) | ((uint32_t)p5 << 8) | p6;

            convResult  = __builtin_custom_inii(0, packed1, packed2);
			pixelCount = pixelCount + 1;

			if(pixelCount == 2){//once we have 2 pixel, we can store them to the output array
				uint8_t pixel1 = ((uint8_t)convResiltBuff ) << 4 |0x0F;

				uint8_t pixel2 = (uint8_t)convResult;
				uint8_t finalResult = (pixel2 | 0xF0) & pixel1;

				outputImg[outputPixelIdx] = finalResult;

				outputPixelIdx = outputPixelIdx + 1;
				pixelCount = 0;
			}else{
				convResiltBuff = convResult; //padding the bottom 4 bits with 0
			}
		}
	}

return outputPixelIdx;
}

int edgeDectect(uint8_t * inputImg, uint8_t * outputImg, int width, int height, int startHeight, int endHeight,int outputPixelIdx){
//	int outputPixelIdx = 0;
	int pixelCount = 0;
	int convResiltBuff = 0;
	int threshold = 11;

	for(int h=startHeight; h<=endHeight-1;h = h+1){
		for(int w=0; w<=width-1;w = w + 1){
			int convResult = 0;
			uint8_t currentPixelVal = 0;

			int pixelAddress1 = ((h-1)*width + (w)) / 2 ;
			int pixelAddress2 = ((h)*width + (w)) / 2;
			int pixelAddress3 = ((h+1)*width + (w)) / 2;

            int p1 = inputImg[pixelAddress1-1];
            int p2 = inputImg[pixelAddress1];

            int p3 = inputImg[pixelAddress2-1];
            int p4 = inputImg[pixelAddress2];

            int p5 = inputImg[pixelAddress3-1];
            int p6 = inputImg[pixelAddress3];

            int val1 = (p1&0x0F);
            int val2 =((p2&0xF0)>>4);
            int val3 = (p2&0x0F);



            int val4 = (p3&0x0F);
            int val5 =((p4&0xF0)>>4);
            int val6 = (p4&0x0F);

            int val7 = (p5&0x0F);
            int val8 =((p6&0xF0)>>4);
            int val9 = (p6&0x0F);



            int sobel1 = -1*val1+ 0*val2 + val3 + -2*val4 + 0*val5 + 2*val6 - val7+ 0*val8 + val9;
			int sobel2 = -1*val1 + -2*val2 + -1*val3 + 0*val4 + 0*val5 + 0*val6 + 1*val7 + 2*val8 + 1*val9;
            convResult = sobel1+sobel2;

            if(convResult < threshold){
            	convResult = 0;
            }


			pixelCount = pixelCount + 1;

			if(pixelCount == 2){//once we have 2 pixel, we can store them to the output array

				uint8_t pixel1 = ((uint8_t)convResiltBuff) << 4 |0x0F;
				uint8_t pixel2 = (uint8_t)convResult;



				uint8_t finalResult = (pixel2 | 0xF0) & pixel1;
				outputImg[outputPixelIdx] = finalResult;

				outputPixelIdx = outputPixelIdx + 1;
				pixelCount = 0;
			}else{
				convResiltBuff = convResult; //padding the bottom 4 bits with 0
			}
		}
	}

return outputPixelIdx;
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
	for(int h=120; h<240; h++){
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


int main()
{


	alt_putstr("Hello from Nios II!\n");

	int counter  = 0;
    float h_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};
    float v_kernel[3] = {1.0f/3, 1.0f/3, 1.0f/3};

    float sobelA_h[3] = {-1,0,1};
    float sobelA_v[3] = {1,2,1};

    float sobelB_h[3] = {1,2,1};
    float sobelB_v[3] = {-1,0,1};

	uint8_t sobelArrX[38400];
	uint8_t sobelArrY[38400];
	uint8_t sobelArrCombined[38400];

	alt_dcache_flush_all();
	uint32_t readBuffer = SHARED_BUFF_1_BASE;

	 while (1){
		 alt_dcache_flush_all();
		 //finding the correct buffer

 		  if(*bufferFlag1 == 0){
 			  readBuffer = SHARED_BUFF_2_BASE;

 		  }else if(*bufferFlag1 == 1){
 			  readBuffer = SHARED_BUFF_1_BASE;
 		  }

		 //img processing

		 if(*quadImgMode == 2){

			 if(*needBlur == 1){
//				 printf("starting blur\n");
//				 convolve_v2(readBuffer,CONV_RESULT_BASE_1,h_kernel,v_kernel,160,120);
				 int topHalfIdx = blurImg(readBuffer,CONV_RESULT_BASE_1,160,120,0,60,0);
//				 printf("topHalf idx is %d \n", topHalfIdx);
				 blurImg(readBuffer,CONV_RESULT_BASE_1,160,120,60,120,4800);

//				 printf("blur finished\n");
			 }
			 if(*needEdgeDetect!=0){
//				 printf("staring edge detection \n");
				 edgeDectect(readBuffer,CONV_RESULT_BASE_2,160,120,0,60,0);
				 edgeDectect(readBuffer,CONV_RESULT_BASE_2,160,120,60,120,4800);
//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrX,sobelA_h,sobelA_v,160,120);
//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrY,sobelB_h,sobelB_v,160,120);
//					combineSobelFilter(sobelArrX,sobelArrY,CONV_RESULT_BASE_2,160,120);
//				printf("end edge detection \n");

			 }

			 *convDone = 1;

		 }else{//single img display
			 if(*needBlur == 1){
//				 convolve_v2(readBuffer,readBuffer,h_kernel,v_kernel,320,240);
				 int fullTopHalf  = blurImg(readBuffer,CONV_RESULT_BASE_1,320,240,0,120,0);
//				 printf("topHalf idx is %d \n", fullTopHalf);
				 blurImg(readBuffer,CONV_RESULT_BASE_1,320,240,120,240,19200);
				 *convDone = 1;
			 }else if(*needEdgeDetect != 0){
//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrX,sobelA_h,sobelA_v,320,240);
//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrY,sobelB_h,sobelB_v,320,240);
//					combineSobelFilter(sobelArrX,sobelArrY,readBuffer,320,240);

					edgeDectect(readBuffer,CONV_RESULT_BASE_2,320,240,0,120,0);
					edgeDectect(readBuffer,CONV_RESULT_BASE_2,320,240,120,240,19200);

//					readBuffer = sobelArrCombined;

					*convDone = 1;

			 }
		 }







	}

	return 0;
	}
