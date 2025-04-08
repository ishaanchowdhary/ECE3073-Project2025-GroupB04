module ECE3073_PROJECT_M1(
	input wire [1:0] KEY,
	input wire CLOCK_50,
	input wire [9:0] SW,
	output wire [18:0] VGA_ADDR,
	output wire [3:0] VGA_R, VGA_G, VGA_B,
	output wire VGA_HS, VGA_VS,
	output [9:0] LEDR,
	
	output write_enable,
	
	
	
		
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
	output [3:0] write_value //value we are writing to the buffer
);

wire VGA_CLK;
//wire write_enable;


PLL_CLK vga_clock(
	.inclk0(CLOCK_50),
	.c0(VGA_CLK)
);

PIXEL_BUFFER pixel_buff(
	.data(write_value), //what we putting in wraddress when write enable is high
	.clock(VGA_CLK),
	.rdaddress(VGA_ADDR[16:0]), //where the VGA is reading the buffer (which pixel address)
	.wraddress(wraddress), // address of what we writing to the buffer
	.wren(!KEY[0]), //write enable
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


Project project_nios(
	.clk_clk          (CLOCK_50),
	.sdram_addr       (DRAM_ADDR),
	.sdram_ba         (DRAM_BA),      			
	.sdram_cas_n      (DRAM_CAS_N),
	.sdram_cke        (DRAM_CKE),
.sdram_cs_n       (DRAM_CS_N),
	.sdram_dq         (DRAM_DQ),
	.sdram_dqm        ({DRAM_UDQM, DRAM_LDQM}),
	.sdram_ras_n      (DRAM_RAS_N),
	.sdram_we_n       (DRAM_WE_N)  ,
	.pixel_address_export(wraddress),
	.pixel_data_export(write_value)
    );



endmodule
