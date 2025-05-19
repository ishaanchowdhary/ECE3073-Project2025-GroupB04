/*
 * system.h - SOPC Builder system and BSP software package information
 *
 * Machine generated for CPU 'nios_SPI_ACCEL' in SOPC Builder design 'PROJECT_SYS_V2'
 * SOPC Builder design path: ../../PROJECT_SYS_V2.sopcinfo
 *
 * Generated: Mon May 19 11:34:11 EST 2025
 */

/*
 * DO NOT MODIFY THIS FILE
 *
 * Changing this file will have subtle consequences
 * which will almost certainly lead to a nonfunctioning
 * system. If you do modify this file, be aware that your
 * changes will be overwritten and lost when this file
 * is generated again.
 *
 * DO NOT MODIFY THIS FILE
 */

/*
 * License Agreement
 *
 * Copyright (c) 2008
 * Altera Corporation, San Jose, California, USA.
 * All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This agreement shall be governed in all respects by the laws of the State
 * of California and by the laws of the United States of America.
 */

#ifndef __SYSTEM_H_
#define __SYSTEM_H_

/* Include definitions from linker script generator */
#include "linker.h"


/*
 * CAM_READY configuration
 *
 */

#define ALT_MODULE_CLASS_CAM_READY altera_avalon_pio
#define CAM_READY_BASE 0x4001070
#define CAM_READY_BIT_CLEARING_EDGE_REGISTER 0
#define CAM_READY_BIT_MODIFYING_OUTPUT_REGISTER 0
#define CAM_READY_CAPTURE 0
#define CAM_READY_DATA_WIDTH 1
#define CAM_READY_DO_TEST_BENCH_WIRING 0
#define CAM_READY_DRIVEN_SIM_VALUE 0
#define CAM_READY_EDGE_TYPE "NONE"
#define CAM_READY_FREQ 50000000
#define CAM_READY_HAS_IN 1
#define CAM_READY_HAS_OUT 0
#define CAM_READY_HAS_TRI 0
#define CAM_READY_IRQ -1
#define CAM_READY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define CAM_READY_IRQ_TYPE "NONE"
#define CAM_READY_NAME "/dev/CAM_READY"
#define CAM_READY_RESET_VALUE 0
#define CAM_READY_SPAN 16
#define CAM_READY_TYPE "altera_avalon_pio"


/*
 * CPU configuration
 *
 */

#define ALT_CPU_ARCHITECTURE "altera_nios2_gen2"
#define ALT_CPU_BIG_ENDIAN 0
#define ALT_CPU_BREAK_ADDR 0x04000820
#define ALT_CPU_CPU_ARCH_NIOS2_R1
#define ALT_CPU_CPU_FREQ 50000000u
#define ALT_CPU_CPU_ID_SIZE 1
#define ALT_CPU_CPU_ID_VALUE 0x00000001
#define ALT_CPU_CPU_IMPLEMENTATION "fast"
#define ALT_CPU_DATA_ADDR_WIDTH 0x1b
#define ALT_CPU_DCACHE_BYPASS_MASK 0x80000000
#define ALT_CPU_DCACHE_LINE_SIZE 32
#define ALT_CPU_DCACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_DCACHE_SIZE 2048
#define ALT_CPU_EXCEPTION_ADDR 0x00000020
#define ALT_CPU_FLASH_ACCELERATOR_LINES 0
#define ALT_CPU_FLASH_ACCELERATOR_LINE_SIZE 0
#define ALT_CPU_FLUSHDA_SUPPORTED
#define ALT_CPU_FREQ 50000000
#define ALT_CPU_HARDWARE_DIVIDE_PRESENT 0
#define ALT_CPU_HARDWARE_MULTIPLY_PRESENT 1
#define ALT_CPU_HARDWARE_MULX_PRESENT 0
#define ALT_CPU_HAS_DEBUG_CORE 1
#define ALT_CPU_HAS_DEBUG_STUB
#define ALT_CPU_HAS_EXTRA_EXCEPTION_INFO
#define ALT_CPU_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define ALT_CPU_HAS_JMPI_INSTRUCTION
#define ALT_CPU_ICACHE_LINE_SIZE 32
#define ALT_CPU_ICACHE_LINE_SIZE_LOG2 5
#define ALT_CPU_ICACHE_SIZE 4096
#define ALT_CPU_INITDA_SUPPORTED
#define ALT_CPU_INST_ADDR_WIDTH 0x1b
#define ALT_CPU_NAME "nios_SPI_ACCEL"
#define ALT_CPU_NUM_OF_SHADOW_REG_SETS 0
#define ALT_CPU_OCI_VERSION 1
#define ALT_CPU_RESET_ADDR 0x00000000


/*
 * CPU configuration (with legacy prefix - don't use these anymore)
 *
 */

#define NIOS2_BIG_ENDIAN 0
#define NIOS2_BREAK_ADDR 0x04000820
#define NIOS2_CPU_ARCH_NIOS2_R1
#define NIOS2_CPU_FREQ 50000000u
#define NIOS2_CPU_ID_SIZE 1
#define NIOS2_CPU_ID_VALUE 0x00000001
#define NIOS2_CPU_IMPLEMENTATION "fast"
#define NIOS2_DATA_ADDR_WIDTH 0x1b
#define NIOS2_DCACHE_BYPASS_MASK 0x80000000
#define NIOS2_DCACHE_LINE_SIZE 32
#define NIOS2_DCACHE_LINE_SIZE_LOG2 5
#define NIOS2_DCACHE_SIZE 2048
#define NIOS2_EXCEPTION_ADDR 0x00000020
#define NIOS2_FLASH_ACCELERATOR_LINES 0
#define NIOS2_FLASH_ACCELERATOR_LINE_SIZE 0
#define NIOS2_FLUSHDA_SUPPORTED
#define NIOS2_HARDWARE_DIVIDE_PRESENT 0
#define NIOS2_HARDWARE_MULTIPLY_PRESENT 1
#define NIOS2_HARDWARE_MULX_PRESENT 0
#define NIOS2_HAS_DEBUG_CORE 1
#define NIOS2_HAS_DEBUG_STUB
#define NIOS2_HAS_EXTRA_EXCEPTION_INFO
#define NIOS2_HAS_ILLEGAL_INSTRUCTION_EXCEPTION
#define NIOS2_HAS_JMPI_INSTRUCTION
#define NIOS2_ICACHE_LINE_SIZE 32
#define NIOS2_ICACHE_LINE_SIZE_LOG2 5
#define NIOS2_ICACHE_SIZE 4096
#define NIOS2_INITDA_SUPPORTED
#define NIOS2_INST_ADDR_WIDTH 0x1b
#define NIOS2_NUM_OF_SHADOW_REG_SETS 0
#define NIOS2_OCI_VERSION 1
#define NIOS2_RESET_ADDR 0x00000000


/*
 * Define for each module class mastered by the CPU
 *
 */

#define __ALTERA_AVALON_JTAG_UART
#define __ALTERA_AVALON_MUTEX
#define __ALTERA_AVALON_NEW_SDRAM_CONTROLLER
#define __ALTERA_AVALON_PIO
#define __ALTERA_AVALON_SPI
#define __ALTERA_NIOS2_GEN2


/*
 * GYRO_INT configuration
 *
 */

#define ALT_MODULE_CLASS_GYRO_INT altera_avalon_pio
#define GYRO_INT_BASE 0x4001080
#define GYRO_INT_BIT_CLEARING_EDGE_REGISTER 0
#define GYRO_INT_BIT_MODIFYING_OUTPUT_REGISTER 0
#define GYRO_INT_CAPTURE 1
#define GYRO_INT_DATA_WIDTH 1
#define GYRO_INT_DO_TEST_BENCH_WIRING 0
#define GYRO_INT_DRIVEN_SIM_VALUE 0
#define GYRO_INT_EDGE_TYPE "RISING"
#define GYRO_INT_FREQ 50000000
#define GYRO_INT_HAS_IN 1
#define GYRO_INT_HAS_OUT 0
#define GYRO_INT_HAS_TRI 0
#define GYRO_INT_IRQ 2
#define GYRO_INT_IRQ_INTERRUPT_CONTROLLER_ID 0
#define GYRO_INT_IRQ_TYPE "EDGE"
#define GYRO_INT_NAME "/dev/GYRO_INT"
#define GYRO_INT_RESET_VALUE 0
#define GYRO_INT_SPAN 16
#define GYRO_INT_TYPE "altera_avalon_pio"


/*
 * KEY10 configuration
 *
 */

#define ALT_MODULE_CLASS_KEY10 altera_avalon_pio
#define KEY10_BASE 0x4001090
#define KEY10_BIT_CLEARING_EDGE_REGISTER 0
#define KEY10_BIT_MODIFYING_OUTPUT_REGISTER 0
#define KEY10_CAPTURE 1
#define KEY10_DATA_WIDTH 2
#define KEY10_DO_TEST_BENCH_WIRING 0
#define KEY10_DRIVEN_SIM_VALUE 0
#define KEY10_EDGE_TYPE "RISING"
#define KEY10_FREQ 50000000
#define KEY10_HAS_IN 1
#define KEY10_HAS_OUT 0
#define KEY10_HAS_TRI 0
#define KEY10_IRQ -1
#define KEY10_IRQ_INTERRUPT_CONTROLLER_ID -1
#define KEY10_IRQ_TYPE "EDGE"
#define KEY10_NAME "/dev/KEY10"
#define KEY10_RESET_VALUE 0
#define KEY10_SPAN 16
#define KEY10_TYPE "altera_avalon_pio"


/*
 * PIXEL_ADDRESS configuration
 *
 */

#define ALT_MODULE_CLASS_PIXEL_ADDRESS altera_avalon_pio
#define PIXEL_ADDRESS_BASE 0x4001060
#define PIXEL_ADDRESS_BIT_CLEARING_EDGE_REGISTER 0
#define PIXEL_ADDRESS_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIXEL_ADDRESS_CAPTURE 0
#define PIXEL_ADDRESS_DATA_WIDTH 17
#define PIXEL_ADDRESS_DO_TEST_BENCH_WIRING 0
#define PIXEL_ADDRESS_DRIVEN_SIM_VALUE 0
#define PIXEL_ADDRESS_EDGE_TYPE "NONE"
#define PIXEL_ADDRESS_FREQ 50000000
#define PIXEL_ADDRESS_HAS_IN 0
#define PIXEL_ADDRESS_HAS_OUT 1
#define PIXEL_ADDRESS_HAS_TRI 0
#define PIXEL_ADDRESS_IRQ -1
#define PIXEL_ADDRESS_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIXEL_ADDRESS_IRQ_TYPE "NONE"
#define PIXEL_ADDRESS_NAME "/dev/PIXEL_ADDRESS"
#define PIXEL_ADDRESS_RESET_VALUE 0
#define PIXEL_ADDRESS_SPAN 16
#define PIXEL_ADDRESS_TYPE "altera_avalon_pio"


/*
 * PIXEL_DATA configuration
 *
 */

#define ALT_MODULE_CLASS_PIXEL_DATA altera_avalon_pio
#define PIXEL_DATA_BASE 0x4001050
#define PIXEL_DATA_BIT_CLEARING_EDGE_REGISTER 0
#define PIXEL_DATA_BIT_MODIFYING_OUTPUT_REGISTER 0
#define PIXEL_DATA_CAPTURE 0
#define PIXEL_DATA_DATA_WIDTH 4
#define PIXEL_DATA_DO_TEST_BENCH_WIRING 0
#define PIXEL_DATA_DRIVEN_SIM_VALUE 0
#define PIXEL_DATA_EDGE_TYPE "NONE"
#define PIXEL_DATA_FREQ 50000000
#define PIXEL_DATA_HAS_IN 0
#define PIXEL_DATA_HAS_OUT 1
#define PIXEL_DATA_HAS_TRI 0
#define PIXEL_DATA_IRQ -1
#define PIXEL_DATA_IRQ_INTERRUPT_CONTROLLER_ID -1
#define PIXEL_DATA_IRQ_TYPE "NONE"
#define PIXEL_DATA_NAME "/dev/PIXEL_DATA"
#define PIXEL_DATA_RESET_VALUE 0
#define PIXEL_DATA_SPAN 16
#define PIXEL_DATA_TYPE "altera_avalon_pio"


/*
 * SDRAM configuration
 *
 */

#define ALT_MODULE_CLASS_SDRAM altera_avalon_new_sdram_controller
#define SDRAM_BASE 0x0
#define SDRAM_CAS_LATENCY 3
#define SDRAM_CONTENTS_INFO
#define SDRAM_INIT_NOP_DELAY 0.0
#define SDRAM_INIT_REFRESH_COMMANDS 2
#define SDRAM_IRQ -1
#define SDRAM_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SDRAM_IS_INITIALIZED 1
#define SDRAM_NAME "/dev/SDRAM"
#define SDRAM_POWERUP_DELAY 100.0
#define SDRAM_REFRESH_PERIOD 15.625
#define SDRAM_REGISTER_DATA_IN 1
#define SDRAM_SDRAM_ADDR_WIDTH 0x19
#define SDRAM_SDRAM_BANK_WIDTH 2
#define SDRAM_SDRAM_COL_WIDTH 10
#define SDRAM_SDRAM_DATA_WIDTH 16
#define SDRAM_SDRAM_NUM_BANKS 4
#define SDRAM_SDRAM_NUM_CHIPSELECTS 1
#define SDRAM_SDRAM_ROW_WIDTH 13
#define SDRAM_SHARED_DATA 0
#define SDRAM_SIM_MODEL_BASE 0
#define SDRAM_SPAN 67108864
#define SDRAM_STARVATION_INDICATOR 0
#define SDRAM_TRISTATE_BRIDGE_SLAVE ""
#define SDRAM_TYPE "altera_avalon_new_sdram_controller"
#define SDRAM_T_AC 5.5
#define SDRAM_T_MRD 3
#define SDRAM_T_RCD 20.0
#define SDRAM_T_RFC 70.0
#define SDRAM_T_RP 20.0
#define SDRAM_T_WR 14.0


/*
 * SW configuration
 *
 */

#define ALT_MODULE_CLASS_SW altera_avalon_pio
#define SW_BASE 0x40010a0
#define SW_BIT_CLEARING_EDGE_REGISTER 0
#define SW_BIT_MODIFYING_OUTPUT_REGISTER 0
#define SW_CAPTURE 0
#define SW_DATA_WIDTH 10
#define SW_DO_TEST_BENCH_WIRING 0
#define SW_DRIVEN_SIM_VALUE 0
#define SW_EDGE_TYPE "NONE"
#define SW_FREQ 50000000
#define SW_HAS_IN 1
#define SW_HAS_OUT 0
#define SW_HAS_TRI 0
#define SW_IRQ -1
#define SW_IRQ_INTERRUPT_CONTROLLER_ID -1
#define SW_IRQ_TYPE "NONE"
#define SW_NAME "/dev/SW"
#define SW_RESET_VALUE 0
#define SW_SPAN 16
#define SW_TYPE "altera_avalon_pio"


/*
 * System configuration
 *
 */

#define ALT_DEVICE_FAMILY "MAX 10"
#define ALT_ENHANCED_INTERRUPT_API_PRESENT
#define ALT_IRQ_BASE NULL
#define ALT_LOG_PORT "/dev/null"
#define ALT_LOG_PORT_BASE 0x0
#define ALT_LOG_PORT_DEV null
#define ALT_LOG_PORT_TYPE ""
#define ALT_NUM_EXTERNAL_INTERRUPT_CONTROLLERS 0
#define ALT_NUM_INTERNAL_INTERRUPT_CONTROLLERS 1
#define ALT_NUM_INTERRUPT_CONTROLLERS 1
#define ALT_STDERR "/dev/jtag_uart_1"
#define ALT_STDERR_BASE 0x40010b0
#define ALT_STDERR_DEV jtag_uart_1
#define ALT_STDERR_IS_JTAG_UART
#define ALT_STDERR_PRESENT
#define ALT_STDERR_TYPE "altera_avalon_jtag_uart"
#define ALT_STDIN "/dev/jtag_uart_1"
#define ALT_STDIN_BASE 0x40010b0
#define ALT_STDIN_DEV jtag_uart_1
#define ALT_STDIN_IS_JTAG_UART
#define ALT_STDIN_PRESENT
#define ALT_STDIN_TYPE "altera_avalon_jtag_uart"
#define ALT_STDOUT "/dev/jtag_uart_1"
#define ALT_STDOUT_BASE 0x40010b0
#define ALT_STDOUT_DEV jtag_uart_1
#define ALT_STDOUT_IS_JTAG_UART
#define ALT_STDOUT_PRESENT
#define ALT_STDOUT_TYPE "altera_avalon_jtag_uart"
#define ALT_SYSTEM_NAME "PROJECT_SYS_V2"


/*
 * TIME_DISPLAY configuration
 *
 */

#define ALT_MODULE_CLASS_TIME_DISPLAY altera_avalon_pio
#define TIME_DISPLAY_BASE 0x4001040
#define TIME_DISPLAY_BIT_CLEARING_EDGE_REGISTER 0
#define TIME_DISPLAY_BIT_MODIFYING_OUTPUT_REGISTER 0
#define TIME_DISPLAY_CAPTURE 0
#define TIME_DISPLAY_DATA_WIDTH 32
#define TIME_DISPLAY_DO_TEST_BENCH_WIRING 0
#define TIME_DISPLAY_DRIVEN_SIM_VALUE 0
#define TIME_DISPLAY_EDGE_TYPE "NONE"
#define TIME_DISPLAY_FREQ 50000000
#define TIME_DISPLAY_HAS_IN 1
#define TIME_DISPLAY_HAS_OUT 0
#define TIME_DISPLAY_HAS_TRI 0
#define TIME_DISPLAY_IRQ -1
#define TIME_DISPLAY_IRQ_INTERRUPT_CONTROLLER_ID -1
#define TIME_DISPLAY_IRQ_TYPE "NONE"
#define TIME_DISPLAY_NAME "/dev/TIME_DISPLAY"
#define TIME_DISPLAY_RESET_VALUE 0
#define TIME_DISPLAY_SPAN 16
#define TIME_DISPLAY_TYPE "altera_avalon_pio"


/*
 * hal configuration
 *
 */

#define ALT_INCLUDE_INSTRUCTION_RELATED_EXCEPTION_API
#define ALT_MAX_FD 4
#define ALT_SYS_CLK none
#define ALT_TIMESTAMP_CLK none


/*
 * input_proc1 configuration
 *
 */

#define ALT_MODULE_CLASS_input_proc1 altera_avalon_pio
#define INPUT_PROC1_BASE 0x4001030
#define INPUT_PROC1_BIT_CLEARING_EDGE_REGISTER 1
#define INPUT_PROC1_BIT_MODIFYING_OUTPUT_REGISTER 0
#define INPUT_PROC1_CAPTURE 1
#define INPUT_PROC1_DATA_WIDTH 1
#define INPUT_PROC1_DO_TEST_BENCH_WIRING 0
#define INPUT_PROC1_DRIVEN_SIM_VALUE 0
#define INPUT_PROC1_EDGE_TYPE "RISING"
#define INPUT_PROC1_FREQ 50000000
#define INPUT_PROC1_HAS_IN 1
#define INPUT_PROC1_HAS_OUT 0
#define INPUT_PROC1_HAS_TRI 0
#define INPUT_PROC1_IRQ 3
#define INPUT_PROC1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define INPUT_PROC1_IRQ_TYPE "EDGE"
#define INPUT_PROC1_NAME "/dev/input_proc1"
#define INPUT_PROC1_RESET_VALUE 0
#define INPUT_PROC1_SPAN 16
#define INPUT_PROC1_TYPE "altera_avalon_pio"


/*
 * jtag_uart_1 configuration
 *
 */

#define ALT_MODULE_CLASS_jtag_uart_1 altera_avalon_jtag_uart
#define JTAG_UART_1_BASE 0x40010b0
#define JTAG_UART_1_IRQ 0
#define JTAG_UART_1_IRQ_INTERRUPT_CONTROLLER_ID 0
#define JTAG_UART_1_NAME "/dev/jtag_uart_1"
#define JTAG_UART_1_READ_DEPTH 64
#define JTAG_UART_1_READ_THRESHOLD 8
#define JTAG_UART_1_SPAN 8
#define JTAG_UART_1_TYPE "altera_avalon_jtag_uart"
#define JTAG_UART_1_WRITE_DEPTH 64
#define JTAG_UART_1_WRITE_THRESHOLD 8


/*
 * mutex_0 configuration
 *
 */

#define ALT_MODULE_CLASS_mutex_0 altera_avalon_mutex
#define MUTEX_0_BASE 0x40010c0
#define MUTEX_0_IRQ -1
#define MUTEX_0_IRQ_INTERRUPT_CONTROLLER_ID -1
#define MUTEX_0_NAME "/dev/mutex_0"
#define MUTEX_0_OWNER_INIT 0
#define MUTEX_0_OWNER_WIDTH 16
#define MUTEX_0_SPAN 8
#define MUTEX_0_TYPE "altera_avalon_mutex"
#define MUTEX_0_VALUE_INIT 0
#define MUTEX_0_VALUE_WIDTH 16


/*
 * output_proc1 configuration
 *
 */

#define ALT_MODULE_CLASS_output_proc1 altera_avalon_pio
#define OUTPUT_PROC1_BASE 0x4001020
#define OUTPUT_PROC1_BIT_CLEARING_EDGE_REGISTER 0
#define OUTPUT_PROC1_BIT_MODIFYING_OUTPUT_REGISTER 0
#define OUTPUT_PROC1_CAPTURE 0
#define OUTPUT_PROC1_DATA_WIDTH 1
#define OUTPUT_PROC1_DO_TEST_BENCH_WIRING 0
#define OUTPUT_PROC1_DRIVEN_SIM_VALUE 0
#define OUTPUT_PROC1_EDGE_TYPE "NONE"
#define OUTPUT_PROC1_FREQ 50000000
#define OUTPUT_PROC1_HAS_IN 0
#define OUTPUT_PROC1_HAS_OUT 1
#define OUTPUT_PROC1_HAS_TRI 0
#define OUTPUT_PROC1_IRQ -1
#define OUTPUT_PROC1_IRQ_INTERRUPT_CONTROLLER_ID -1
#define OUTPUT_PROC1_IRQ_TYPE "NONE"
#define OUTPUT_PROC1_NAME "/dev/output_proc1"
#define OUTPUT_PROC1_RESET_VALUE 0
#define OUTPUT_PROC1_SPAN 16
#define OUTPUT_PROC1_TYPE "altera_avalon_pio"


/*
 * spi_0 configuration
 *
 */

#define ALT_MODULE_CLASS_spi_0 altera_avalon_spi
#define SPI_0_BASE 0x4001000
#define SPI_0_CLOCKMULT 1
#define SPI_0_CLOCKPHASE 1
#define SPI_0_CLOCKPOLARITY 1
#define SPI_0_CLOCKUNITS "Hz"
#define SPI_0_DATABITS 8
#define SPI_0_DATAWIDTH 16
#define SPI_0_DELAYMULT "1.0E-9"
#define SPI_0_DELAYUNITS "ns"
#define SPI_0_EXTRADELAY 0
#define SPI_0_INSERT_SYNC 0
#define SPI_0_IRQ 1
#define SPI_0_IRQ_INTERRUPT_CONTROLLER_ID 0
#define SPI_0_ISMASTER 1
#define SPI_0_LSBFIRST 0
#define SPI_0_NAME "/dev/spi_0"
#define SPI_0_NUMSLAVES 2
#define SPI_0_PREFIX "spi_"
#define SPI_0_SPAN 32
#define SPI_0_SYNC_REG_DEPTH 2
#define SPI_0_TARGETCLOCK 5000000u
#define SPI_0_TARGETSSDELAY "0.0"
#define SPI_0_TYPE "altera_avalon_spi"

#endif /* __SYSTEM_H_ */
