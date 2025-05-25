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

/*
* Processor 3 (P3) is the fourth processor in the system.
 * It is the fourth processor out of the four processors in the system.
 * Responsibilities:
 * - a helper processor that can be used for debugging or other tasks.
 * - currenly it is used to display the status of the system.
*/

// -- all our memory addresses and flags noted here --
int *display1Ready = (int*)0x03600010;

int *bufferFlag1 = (int*)0x03200020; //use as buffer idx
int *frame1Ready = (int*)0x03200030;


int *convDone = (int*) 0x03200040;
int *convStart = (int*)0x03200050;

int *sharedMsgBuff = (int*)0x03500000;

int *needBlur = (int*)0x03200070;
int *quadImgMode = (int*)0x0320080;
int *needEdgeDetect = (int*)0x03200000;

/*
 * our main function for P3
 * it is used to display the status of the system.
 * prints to the console the blur, quad image mode and edge detection status.
 * It is a simple event loop that never exits.
*/
int main()
{ 
  alt_putstr("Hello from Nios II!\n");

  /* Event loop never exits. */
  while (1){
	  printf("need blur = %d , quadImgMode = %d, *needEdgeDetect = %d \n",*needBlur,*quadImgMode,*needEdgeDetect);
  }

  return 0;
}
