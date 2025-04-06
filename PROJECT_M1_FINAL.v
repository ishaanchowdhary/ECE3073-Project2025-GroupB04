module PROJECT_M1_FINAL(
	input wire [1:0] KEY,
	input wire CLOCK_50,
	input wire [9:0] SW,
	output wire [18:0] VGA_ADDR,
	output wire [3:0] VGA_R, VGA_G, VGA_B,
	output wire VGA_HS, VGA_VS,
	output [9:0] LEDR,
	input wire reset_reset_n,
	
	output write_enable,
	// ---------camera IO-------------------//
	input  wire CAM_READY,
	
	//------------hex IO-----------------------//
	output wire [7:0] HEX0,
	output wire [7:0] HEX1,
	output wire [7:0] HEX2,
	output wire [7:0] HEX3,
	
	
	//output wire [23:0] HEX5,
	//output wire [23:0] HEX6,
	
	//------------Timer IO-----------------------//
	output wire time_enable,
	output wire [31:0] count_ext,
	
	
		
	// ---- SDRAM IO ---- //
	output	[12:0]		DRAM_ADDR,
	output	[1:0]			DRAM_BA,
	output				DRAM_CAS_N,
	output				DRAM_CKE,
	output				DRAM_CLK,
	output				DRAM_CS_N,
	inout		[15:0]		DRAM_DQ,
	output				DRAM_LDQM,
	output				DRAM_UDQM,
	output				DRAM_RAS_N,
	output				DRAM_WE_N,
	// ----- PIXIEL BUFFER ----//

	output [16:0] wraddress,	// same as pixel address export
	output wire [3:0] pixel_out, //output from the buffer to the VGA
	output [3:0] write_value, //value we are writing to the buffer
	
	// ------ SPI IO ------//
	//input SPI_MISO,
	//output SPI_MOSI,
	//output SPI_SCLK,
	//output SPI_N,
	
	inout wire [10:0] GPIO
	
//	input GPIO[7],
//	output GPIO[8],
//	output GPIO[9],
//	input GPIO[2]
//	
//	
	
);

//		input  wire        spi_external_MISO,    //  spi_external.MISO
//		output wire        spi_external_MOSI,    //              .MOSI
//		output wire        spi_external_SCLK,    //              .SCLK
//		output wire        spi_external_SS_n,    //              .SS_n
//		input  wire [9:0]  switches_export       //      switches.export

//	 assign GPIO[7] = SPI_MISO;
//	 assign GPIO[8] = SPI_MOSI;
//	 assign GPIO[9] = SPI_SCLK;
//	 
//	 assign GPIO[2] = CAM_READY;



wire VGA_CLK;
//wire write_enable;


PLL_CLK vga_clock(
	.inclk0(CLOCK_50),
	.c0(VGA_CLK)
);


SDRAM_PLL sdram_pll(
	.inclk0(CLOCK_50),
	.c0(DRAM_CLK)
);


//
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
	.VGA_CLK(VGA_CLK), //todo need to fix this to 25MHz
	.VGA_ADDR(VGA_ADDR),
	.VGA_R(VGA_R),
	.VGA_G(VGA_G),
	.VGA_B(VGA_B),
	.VGA_HS(VGA_HS),
	.VGA_VS(VGA_VS)

);

//		input  wire        cam_ready_export,     //     cam_ready.export
//		input  wire        clk_clk,              //           clk.clk
//		output wire [23:0] first_hex_export,     //     first_hex.export
//		input  wire [1:0]  keys_export,          //          keys.export
//		output wire [9:0]  leds_export,          //          leds.export
//		output wire [16:0] pixel_address_export, // pixel_address.export
//		output wire [3:0]  pixel_data_export,    //    pixel_data.export

//		output wire [12:0] sdram_addr,           //         sdram.addr
//		output wire [1:0]  sdram_ba,             //              .ba
//		output wire        sdram_cas_n,          //              .cas_n
//		output wire        sdram_cke,            //              .cke
//		output wire        sdram_cs_n,           //              .cs_n
//		inout  wire [15:0] sdram_dq,             //              .dq
//		output wire [1:0]  sdram_dqm,            //              .dqm
//		output wire        sdram_ras_n,          //              .ras_n
//		output wire        sdram_we_n,           //              .we_n

//		output wire [23:0] second_hex_export,    //    second_hex.export
//		input  wire [9:0]  switches_export       //      switches.export


//		input  wire        spi_external_MISO,    //  spi_external.MISO
//		output wire        spi_external_MOSI,    //              .MOSI
//		output wire        spi_external_SCLK,    //              .SCLK
//		output wire        spi_external_SS_n,    //              .SS_n
//		input  wire [9:0]  switches_export       //      switches.export


PROJECT_SYS_V2 project_nios(
	.clk_clk          (CLOCK_50),
//	.reset_reset_n(1'b0),
	//------------------camera-------
	.cam_ready_export(GPIO[2]),
	//---------------hex display---------
	//.first_hex_export(HEX5),
	//.second_hex_export(HEX6),
	//-------------switches----------
	.switches_export(SW),
	//------------keys------------
	.keys_export(KEY),
	.leds_export(LEDR),
	
	//---------pixel buffer IO------------
	.pixel_address_export(wraddress),
	.pixel_data_export(write_value),	
	
	//-----------SDRAM IO--------------
	.sdram_addr       (DRAM_ADDR),
	.sdram_ba         (DRAM_BA),      			
	.sdram_cas_n      (DRAM_CAS_N),
	.sdram_cke        (DRAM_CKE),
	.sdram_cs_n       (DRAM_CS_N),
	.sdram_dq         (DRAM_DQ),
	.sdram_dqm        ({DRAM_UDQM, DRAM_LDQM}),
	.sdram_ras_n      (DRAM_RAS_N),
	.sdram_we_n       (DRAM_WE_N)  ,
	
	//SPI IO
	.spi_external_MISO(GPIO[7]),
	.spi_external_MOSI(GPIO[8]),    //              .MOSI
	.spi_external_SCLK(GPIO[9]),    //              .SCLK
	.spi_external_SS_n(GPIO[5]),    //              .SS_n
	
	.time_enable_export		(time_enable)

    );
	 
//	 assign GPIO[7] = SPI_MISO;
//	 assign GPIO[8] = SPI_MOSI;
//	 assign GPIO[9] = SPI_SCLK;
//	 assign GPIO[5] = SPI_N;
//	 
//	 assign CAM_READY=GPIO[2] ;
	 
	 assign GPIO[10] = 1'bz;
	 assign GPIO[6] = 1'bz;
	 assign GPIO[4] = 1'bz;
	 assign GPIO[3] = 1'bz;
	 assign GPIO[1] = 1'bz;
	 assign GPIO[0] = 1'bz;

	 

	 
// TASK 4	 



// Instantiate the timer and display modules:


frame_timer frame(
.clk				(CLOCK_50),
.reset_n			(KEY[0]),
.time_enable	(time_enable),
.count_ext		(count_ext)

);

fps_display display(
.count_ext  (count_ext),
.hex0	 		(HEX0[6:0]),			  // Least significant digit
.hex1	 		(HEX1[6:0]),	
.hex2 	 	(HEX2[7:0]),
.hex3 	 	(HEX3[6:0])			  // Most significant digit


);



	 











endmodule

// Timer Function to calculate frame rate
// Create signal that turns on and off the timer
// Output a 32 bit unsigned value for the time stamp	 

module frame_timer (
    input clk,          // FPGA system clock (e.g., 50 MHz)
    input reset_n,      // Active-low reset
	 input time_enable,
    output reg [31:0] count_ext // 32-bit microsecond counter output
);
    //parameter SYS_CLK_FREQ = 50000000;  // 50 MHz clock
    //parameter USEC_TICK    = SYS_CLK_FREQ / 1000000; // Divider for 1 µs tick
	 
	 reg [31:0] count;
	 
	 
		 
	always @(posedge clk or negedge reset_n) begin
		 if (!reset_n) begin
			  count <= 32'd0;
			  count_ext <= 32'd0;
		 end else begin
			  case (time_enable)
					1'd1: begin
						 // Increment count on each clock cycle
						 count <= count + 1;
					end
					1'd0: begin
						 // Save the value of count
						 count_ext <= count;
						 // Reset count when enable is turned off
						 count <= 32'd0;
					end
					default: count <= 32'd0;
			  endcase
		 end
	end
	



	
	 
endmodule



// Module to display The frame rate in HEX[3:0]

module fps_display (
    input [31:0] count_ext,
    output [6:0] hex0, // Least significant digit
    output [6:0] hex1,
    output [7:0] hex2, // Decimal point will be handled separately
    output [6:0] hex3  // Most significant digit
);

    // Now we need to convert the count value into fps
	 // 1 pulse is 20ns and so fps = (10^8) / (2xcount)
	 // times it by 100 
	 wire [31:0] fps;
	 assign fps = 10000000000/(2*count_ext);
	 
	 
	 
	 

    wire [3:0] tens, ones, tenths, hundredths; 



  // Extract BCD digits using division (simple method)
	 assign tens 		 = (fps / 1000) % 10;
    assign ones  	    = (fps / 100) % 10;
    assign tenths  	 = (fps / 10) % 10;
	 assign hundreths  = fps  % 10;
    
	 
	 // Instantiate the hex_decoder for each digit
    hex_decoder hex3_decoder (
        .digit(tens), 
        .hex_decoder(hex3)
    );
    
    hex_decoder hex2_decoder (
        .digit(ones), 
        .hex_decoder(hex2[6:0])
    );

    hex_decoder hex1_decoder (
        .digit(tenths), 
        .hex_decoder(hex1)
    );

    hex_decoder hex0_decoder (
        .digit(hundredths), 
        .hex_decoder(hex0)
    );
 
	 assign hex2[7] = 1'b1; // decimal point is gonna always be on
	 
endmodule



// HEX Decoder

module hex_decoder(
    input [3:0] digit,        // 4-bit input for digit
    output reg [6:0] hex_decoder // 7-bit output for 7-segment display
);

    // Always block to handle the case statement
    always @(*) begin
        case (digit)
            4'd0: hex_decoder = 7'b1000000;
            4'd1: hex_decoder = 7'b1111001;
            4'd2: hex_decoder = 7'b0100100;
            4'd3: hex_decoder = 7'b0110000;
            4'd4: hex_decoder = 7'b0011001;
            4'd5: hex_decoder = 7'b0010010;
            4'd6: hex_decoder = 7'b0000010;
            4'd7: hex_decoder = 7'b1111000;
            4'd8: hex_decoder = 7'b0000000;
            4'd9: hex_decoder = 7'b0010000;
            default: hex_decoder = 7'b1111111; // Blank
        endcase
    end

endmodule































