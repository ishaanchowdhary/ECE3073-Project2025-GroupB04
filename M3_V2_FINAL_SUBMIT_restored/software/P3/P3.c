#include "sys/alt_stdio.h"
#include <stdio.h>
#include <stdlib.h>
#include "stdint.h"
#include "io.h"
#include "system.h"
#include "system.h"
#include "sys/alt_irq.h"
#include "altera_avalon_mutex.h"

#include <stdio.h>

int *display1Ready = (int*)0x03600010;

int *bufferFlag1 = (int*)0x03200020; //use as buffer idx
int *frame1Ready = (int*)0x03200030;


int *convDone = (int*) 0x03200040;
int *convStart = (int*)0x03200050;

int *subResultReady = (int*) 0x03200060;

int *startSubProcess = (int*) 0x03200090;

int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;

//int *sharedMsgBuff = (int*)0x03ff0000;
#define SHARED_BUFF_1_BASE  0x03300100
#define SHARED_BUFF_2_BASE  0x033096ff

#define CONV_RESULT_BASE_1 0x03400000
#define CONV_RESULT_BASE_2 0x03410000

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

int main()
{ 
  alt_putstr("Hello from Nios II!\n");


  uint32_t readBuffer = SHARED_BUFF_1_BASE;

  /* Event loop never exits. */
  while (1){
//	  	  printf("need blur = %d , quadImgMode = %d, *needEdgeDetect = %d \n",*needBlur,*quadImgMode,*needEdgeDetect);
	  	  alt_dcache_flush_all();
		 //finding the correct buffer

		  if(*bufferFlag1 == 0){
			  readBuffer = SHARED_BUFF_2_BASE;

		  }else if(*bufferFlag1 == 1){
			  readBuffer = SHARED_BUFF_1_BASE;
		  }


			 if(*quadImgMode == 2){
//				 printf("in quad mode, waiting\n");
				 if(*needEdgeDetect!=0){
					 edgeDectect(readBuffer,CONV_RESULT_BASE_2,160,120,0,120,0);
					 *subResultReady = 1;
				 }

			 }else{//single img display
				 if((*needBlur == 1) && (*startSubProcess == 1)){

	//				 convolve_v2(readBuffer,readBuffer,h_kernel,v_kernel,320,240);
//					*subResultReady = 0;
//					alt_dcache_flush_all();
					 blurImg(readBuffer,CONV_RESULT_BASE_2,320,240,120,240,19200);
					 *subResultReady = 1;

				 }else if(*needEdgeDetect != 0){
	//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrX,sobelA_h,sobelA_v,320,240);
	//					convolve_v2(SHARED_BUFF_1_BASE,sobelArrY,sobelB_h,sobelB_v,320,240);
	//					combineSobelFilter(sobelArrX,sobelArrY,readBuffer,320,240);

						edgeDectect(readBuffer,CONV_RESULT_BASE_2,320,240,0,120,0);
						edgeDectect(readBuffer,CONV_RESULT_BASE_2,320,240,120,240,19200);

	//					readBuffer = sobelArrCombined;

						*convDone = 1;
						*startSubProcess = 0;

				 }
			 }







  }

  return 0;
}
