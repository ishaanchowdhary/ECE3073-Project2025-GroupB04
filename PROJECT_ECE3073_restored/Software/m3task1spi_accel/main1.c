#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include <stdlib.h>
#include <stdint.h>
#include "altera_avalon_mutex.h"
int main()
{ 
  alt_putstr("Hello from Nios II!\n");

  /* Event loop never exits. */
  while (1);

  return 0;
}
