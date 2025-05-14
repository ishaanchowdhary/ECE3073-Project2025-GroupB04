// Student IDs: 33115303, 33867860, 33893012, 3311316

module PROJECT_M1_FINAL(
	
	// General I/O
	
	input wire [1:0] KEY,
	input wire CLOCK_50,
	input wire [9:0] SW,
	output wire [18:0] VGA_ADDR,
	output wire [3:0] VGA_R, VGA_G, VGA_B,
	output wire VGA_HS, VGA_VS,
	output [9:0] LEDR,
	input wire reset_reset_n,
	inout wire [10:0] GPIO,
	
	// Camera I/O
	
	input wire CAM_READY,
	
	// Hex I/O
	
	output wire [7:0] HEX0,
	output wire [7:0] HEX1,
	output wire [7:0] HEX2,
	output wire [7:0] HEX3,
	output wire [7:0] HEX4,
	output wire [7:0] HEX5,
		
	// SDRAM I/O
	
	output	[12:0]	DRAM_ADDR,
	output	[1:0]		DRAM_BA,
	output				DRAM_CAS_N,
	output				DRAM_CKE,
	output				DRAM_CLK,
	output				DRAM_CS_N,
	inout		[15:0]	DRAM_DQ,
	output				DRAM_LDQM,
	output				DRAM_UDQM,
	output				DRAM_RAS_N,
	output				DRAM_WE_N,
	
	// PIXIEL BUFFER Interface

	output [16:0] wraddress,	// same as pixel address export
	output wire [3:0] pixel_out, //output from the buffer to the VGA
	output [3:0] write_value, //value we are writing to the buffer
	
	//Accelerometer 
	output GSENSOR_SCLK,
	inout GSENSOR_SDI,
	inout GSENSOR_SDO,
	output GSENSOR_CS_N, 
	input [2:1] GSENSOR_INT
	
	
	
);

// Wires and Assigns


// wire for spi
wire spi_clk;
wire spi_miso;
wire spi_mosi;
wire [1:0] spi_cs; 

// Wires for timer, hex and VGA clock
wire [31:0] count_ext;
wire VGA_CLK;
wire [23:0] first_hex_data;
wire [23:0] second_hex_data;

// Assign HEX displays (each HEX takes 7 bits)
assign HEX0 = first_hex_data[7:0];     // Ones
assign HEX1 = first_hex_data[15:8];    // Tenths
assign HEX2 = first_hex_data[23:16];   // Hundredths
assign HEX3 = second_hex_data[7:0];    // Tens
assign HEX4[7:0] = 8'b11111111; 
assign HEX5[7:0] = 8'b11111111;

// Assign GPIOs
assign GPIO[10] = 1'bz;
assign GPIO[6] = 1'bz;
assign GPIO[4] = 1'bz;
assign GPIO[3] = 1'bz;
assign GPIO[1] = 1'bz;
assign GPIO[0] = 1'bz;

// SPI assignments for both the gyro and ESP-cam 
// for spi
assign GPIO[8] = spi_mosi;
assign GPIO[9] = spi_clk; 
assign GPIO[5] = spi_cs[0];  

// now for the gyro
assign GSENSOR_SDI = spi_mosi;
assign GSENSOR_SCLK = spi_clk; 
assign GSENSOR_CS_N = spi_cs[1]; 

assign spi_miso = (spi_cs[0] == 1'b0) ? GPIO[7] : (spi_cs[1] == 1'b0)  ? GSENSOR_SDO : 1'bz; 
// Instantiate Modules

PLL_CLK vga_clock(
	.inclk0(CLOCK_50),
	.c0(VGA_CLK)
);


PIXEL_BUFFER pixel_buff(
	.data(write_value), //what we putting in wraddress when write enable is high
	.clock(VGA_CLK),
	.rdaddress(VGA_ADDR[16:0]), //where the VGA is reading the buffer (which pixel address)
	.wraddress(wraddress), // address of what we writing to the buffer
	.wren(1'b1), //write enable
	.q(pixel_out)
);

vga_controller vga_inst(

	.VGA_DATA(pixel_out),
	.VGA_CLK(VGA_CLK),
	.VGA_ADDR(VGA_ADDR),
	.VGA_R(VGA_R),
	.VGA_G(VGA_G),
	.VGA_B(VGA_B),
	.VGA_HS(VGA_HS),
	.VGA_VS(VGA_VS)

);

usec_timer timer(
.clk				(CLOCK_50),
.reset			(KEY[0]),
.count_ext		(count_ext)

);

PROJECT_SYS_V2 project_nios(
	.clk_clk          (CLOCK_50),
//	.reset_reset_n(1'b0),
	// Camera
	.cam_ready_export(GPIO[2]),
	// HEX Display
	.first_hex_export(first_hex_data),		// Use this for the first 3 HEXS (Ones, Tenths and Hundreths)
	.second_hex_export(second_hex_data),		// Use this for the 4th HEX (tens place)
	// Switches
	.switches_export(SW),
	// Keys
	.keys_export(KEY),
	.leds_export(LEDR),
	
	// Pixel buffer I/O
	.pixel_address_export(wraddress),
	.pixel_data_export(write_value),	
	
	// SDRAM I/O 
	.sdram_pll_clk    (DRAM_CLK),
	.sdram_addr       (DRAM_ADDR),
	.sdram_ba         (DRAM_BA),      			
	.sdram_cas_n      (DRAM_CAS_N),
	.sdram_cke        (DRAM_CKE),
	.sdram_cs_n       (DRAM_CS_N),
	.sdram_dq         (DRAM_DQ),
	.sdram_dqm        ({DRAM_UDQM, DRAM_LDQM}),
	.sdram_ras_n      (DRAM_RAS_N),
	.sdram_we_n       (DRAM_WE_N)  ,
	
	// SPI I/O
	.spi_external_MISO(spi_miso),
	.spi_external_MOSI(spi_mosi),    // .MOSI
	.spi_external_SCLK(spi_clk),    // .SCLK
	.spi_external_SS_n(spi_cs),    // .SS_n
	.gyro_int_export(GSENSOR_INT[2:1]),
	// COUNT
	.time_display_export		(count_ext)			// 32 bit time display

    );

endmodule



