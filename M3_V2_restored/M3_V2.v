module M3_V2 (
	// Generic I/O
	input 					CLOCK_50,
	input wire 	[1:0]		KEY,
	input wire 	[9:0] 	SW,
	output wire [18:0] 	VGA_ADDR,
	output wire [3:0] 	VGA_R, VGA_G, VGA_B,
	output wire 			VGA_HS, VGA_VS,
	output 		[9:0]		LEDR,
	input wire 				reset_reset_n,
	inout wire 	[10:0] 	GPIO,
	
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
	
	// Camera I/O
	input wire CAM_READY,
	
	// HEX I/O
	output wire [7:0] HEX0,
	output wire [7:0] HEX1,
	output wire [7:0] HEX2,
	output wire [7:0] HEX3,
	output wire [7:0] HEX4,
	output wire [7:0] HEX5,
	
	// PIXEL BUFFER INTERFACE
	output 		[16:0] 	wraddress,		// same as pixel address export
	output wire [3:0] 	pixel_out, 		//output from the buffer to the VGA
	output 		[3:0] 	write_value, 	//value we are writing to the buffer 
	
	// Gyroscope I/O  
	output 		GSENSOR_SCLK, 
	inout 		GSENSOR_SDI, 
	inout 		GSENSOR_SDO,
	output 		GSENSOR_CS_N, 
	input [2:1] GSENSOR_INT

);

// Wires and Assigns
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

// SPI assignment for gyro and Camera  
assign GPIO[8] = spi_mosi; 
assign GPIO[9] = spi_clk; 
assign GPIO[5] = spi_cs[0];  

assign GSENSOR_SDI = spi_mosi; 
assign GSENSOR_SCLK = spi_clk; 
assign GSENSOR_CS_N = spi_cs[1]; 

assign spi_miso = (spi_cs[0] == 1'b0) ? GPIO[7] : (spi_cs[1] == 1'b0) ? GSENSOR_SDO : 1'bz; 

// Assign Processor Communication
wire p0_out,p0_in,p1_out,p1_in;

assign p1_in = p0_out;
assign p0_in = p1_out;

// Instantiate Modules

VGA_PLL_V2 vga_clk(
	.inclk0(CLOCK_50),
	.c0(VGA_CLK)
);

SDRAM_PLL_V2 pll0 (
	.inclk0(CLOCK_50),
	.c0(DRAM_CLK)

);

PIXEL_BUFFER_V2 pixel_buff(
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


MUTIPROCESSOR_V2 u0 (
		.clk_clk          (CLOCK_50),
		//interrupt PIO
		.p0_out_export      (p0_out),      // p0_out.export
		.p0_in_export       (p0_in),       // p0_in.export
		.p1_out_export      (p1_out),      // p1_out.export
		.p1_in_export       (p1_in),       // p1_in.export
		
		// Camera
		.cam_ready_export(GPIO[2]),
		// HEX Display
		.first_hex_export(first_hex_data),		// Use this for the first 3 HEXS (Ones, Tenths and Hundreths)
		.second_hex_export(second_hex_data),	// Use this for the 4th HEX (tens place)
		// Switches
		.switches_export(SW),
		// Keys
		.keys_export(KEY),
		.leds_export(LEDR),
	
		// Pixel buffer I/O
		.pixel_address_export(wraddress),
		.pixel_data_export(write_value),	
	
		//SDRAM IO
		.sdram_addr    		(DRAM_ADDR),
		.sdram_ba      		(DRAM_BA),      		
		.sdram_cas_n   		(DRAM_CAS_N),
		.sdram_cke     		(DRAM_CKE),
		.sdram_cs_n    		(DRAM_CS_N),
		.sdram_dq      		(DRAM_DQ),
		.sdram_dqm     		({DRAM_UDQM, DRAM_LDQM}),
		.sdram_ras_n   		(DRAM_RAS_N),
		.sdram_we_n    		(DRAM_WE_N),
		
		//SPI IO
		.spi_external_MISO(spi_miso),
		.spi_external_MOSI(spi_mosi),    // .MOSI
		.spi_external_SCLK(spi_clk),    	// .SCLK
		.spi_external_SS_n(spi_cs),    	// .SS_n 
		.gyro_int_export(GSENSOR_INT[2]),
	
	//count
		.time_display_export		(count_ext)		// 32 bit time display
);
	 
endmodule


// Microsecond Timer Module

module usec_timer (
    input 					clk,      // FPGA system clock (e.g., 50 MHz)
    input 					reset,    // Active-low reset
    output reg [31:0]	count_ext // 32-bit microsecond counter output
);
// Runs a counter of micro seconds that have occured since the program has started

reg [31:0] clk_checker;

always @(posedge clk or negedge reset) begin
	
	if (~reset) begin
		// Reset both the values
		count_ext <= 32'b1;
		clk_checker <= 32'b0;
	end
	else begin
		if(clk_checker == 49) begin 			// check if the checker is 50 yet
			count_ext <= 1 + count_ext; 		// add 1us to the tally
			clk_checker <= 32'b0; 				// reset the checker to be ready for the next cycle
		end
		else begin
			clk_checker <= 1 + clk_checker;  // increment the counter until hits 50 (i.e. converts 50MHz to 1us)
		end
	end
end

endmodule
