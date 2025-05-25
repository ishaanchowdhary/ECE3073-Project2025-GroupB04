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

int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;


int main()
{ 
  alt_putstr("Hello from Nios II!\n");

  /* Event loop never exits. */
  while (1){
	  printf("need blur = %d , quadImgMode = %d, *needEdgeDetect = %d \n",*needBlur,*quadImgMode,*needEdgeDetect);
  }

  return 0;
}
