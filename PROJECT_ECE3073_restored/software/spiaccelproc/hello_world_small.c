// Camera and Accelerometer Processor

#include "sys/alt_stdio.h"
#include "system.h"
#include "io.h"
#include <stdlib.h>
#include <stdint.h>
#include <altera_avalon_pio_regs.h>
#include "altera_avalon_mutex.h"

#define SDRAM_BASE_ADDRESS 0x00000000
#define IMAGE_SIZE (320*240/2)//each pixel 4 bits, top 4 bits = first pixel

#define PIXEL_ADDRESS_BASE_val 0x4041080
#define PIXEL_DATA_BASE_val 0x4041070

//==========SPI constants==========
#define SPI_CONTROLLER_BASE 0x4041000
// SPI CHIP SELECTS
#define CS_ACCEL 1
#define CS_CAM 0

void *context;
alt_mutex_dev* mutex;
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
	TAP_THRES, 0x20,
	TAP_AXES, 0x07,
	LATENT, 0x85,
	DUR, 0x40,
	WINDOW, 0xc0,
	BW_RATE, 0x0a,
	INT_ENABLE, 0x60,
	INT_MAP, 0x20,
	POWER_CONTROL, 0x08
};

// shared buffer
int *sharedMsgBuff = (int*)0x03500000;
// Interrupt Flags Define
volatile int tap_flag = 0;	// Double Tap Interrupt Flag


// Function Declarations
void gyro_isr(void * context);
void gyro_detect_tap(volatile int *tap_flag, int *counter);
int display_select_configuration(alt_16 yData);
alt_16 gyro_process_data(alt_u8 readX, alt_u8 readY, alt_u8 readZ, alt_16 xData, alt_16 yData, alt_16 zData);

int main() {
	alt_putstr("Camera/Accelerometer Processor Initialised\n");

	// Accelerometer Setup
	alt_u8 gyro_data_in, gyro_data_out, regData;
	alt_u8 readX = READ_X_AXIS;
	alt_u8 readY = READ_Y_AXIS;
	alt_u8 readZ = READ_Z_AXIS;
	alt_16 xData, yData, zData;
	alt_u8 isrRes = 0xff;

	for (int i = 0; i < CONFIG_LENGTH; i += 2) {
		alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_ACCEL, 2, gyro_config + i, 0, &gyro_data_out, 0);
	}

	void* context = (void *) &isrRes;
	IOWR(GYRO_INT_BASE, 3, 0);
	IOWR(GYRO_INT_BASE, 2, 0x1);
	int gyroISR = alt_ic_isr_register(GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID, GYRO_INT_IRQ, gyro_isr, context, 0x0);

	// Initialising Double Tap Counter for updating Display
	int counter = 0;

	while(1){
		// pseudocode
		// Required functionality: take bit from display proc to inform which display mode is being used (0 for quad, 1 for single)
		// if Quad:
		// int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0 ,1,sendBuffPtrSmall,9600,&rxArrSmall,0); //SPI for SMALL res
		// quad_config_mode = display_select_configuration(yData);
		// send quad_config_mode over sdram
		// if Single:
		// int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, 0, 1, sendBuffPtrFull, 38400, &rxArr, 0); // SPI full res

		//psuedo code attempt:
		// need to get the data from the other soure
		altera_avalon_mutex_lock(mutex,1);
		int display_mode = *sharedMsgBuff;
		altera_avalon_mutex_unlock(mutex);
		//2. handle acceleroometer tap
		gyro_detect_tap(&tap_flag, &counter);

		// 3. read gyroscope axis data dn get yData
		yData = gyro_process_data(readX, readY, readZ, xData, yData, zData);


		if(display_mode == 0) {

			alt_u8 sendBuffPtrSmall[9600];
			alt_u8 rxArrSmall[9600];

			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_CAM, 1, sendBuffPtrSmall, 9600, rxArrSmall, 0);
			int config_mode = display_select_configuration(yData);
			send_msg(config_mode);
			IOWR_32DIRECT(SDRAM_BASE_ADDRESS, 0, config_mode);




		} else if (display_mode = 1) {
			alt_u8 sendBuffPtrFull[38400];
			alt_u8 rxArr[38400];

			int status = alt_avalon_spi_command(SPI_CONTROLLER_BASE, CS_CAM, 1, sendBuffPtrFull, 38400, rxArr, 0);


		}

		gyro_data_in = INT_SOURCE | 0x80;
		alt_avalon_spi_command(SPI_CONTROLLER_BASE, 1, 1, &gyro_data_in, 1, &regData, 0x0);
	}

	return 0;
}


// Interrupt service routine (ISR) for accelerometer interrupt
void gyro_isr(void * context) {
	IOWR_ALTERA_AVALON_PIO_EDGE_CAP(GYRO_INT_BASE, 0); //clear interrupt
	IOWR(GYRO_INT_BASE, 3, 0);
	tap_flag = 1; //set the tap flag when an interrupt is triggered
}

void gyro_detect_tap(volatile int *tap_flag, int *counter) {
	// Prints result of triggering accelerometer double tap interrupt and adds to counter

	// Print accelerometer double tap result
	if (*tap_flag == 1) {
		printf("\n\nDouble tap detected!\n\n");
		*tap_flag = 0;
		*counter = *counter + 1;
	}

	// Reset Counter if over 3
	if (*counter >= 4) {
		*counter = 0;
	}

}

int display_select_configuration(alt_16 yData) {
	// selects the configureation mode for Quad Display (loaded into other proc)
	if (yData >= -265 && yData < -127) {
        int config_mode = 0;
    }
    else if (yData >= -127 && yData < 0) {
        int config_mode = 1;
    }
    else if (yData >= 0 && yData < 127) {
        int config_mode = 2;
    }
    else if (yData >= 127 && yData <= 265) {
        int config_mode = 3;
    }
	return 0;
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

void send_msg(int msg) {
	altera_avalon_mutex_lock(mutex, 1);

	*sharedMsgBuff = msg;

	alt_dcache_flush_all();
	altera_avalon_mutex_unlock(mutex);

	IOWR(OUTPUT_PROC1_BASE, 0, 1);
	IOWR(OUTPUT_PROC1_BASE, 0, 0);

	alt_printf("SPI Processor: sneding %d to other processor\n", msg);

}
