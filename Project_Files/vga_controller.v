// *********************************
//
// VGA Controller Module
// Made for students to use during the ECE3073 Project
// 
// Author(s): ECE3073 staff
// Last Edited: 25/02/2024
//
// Copyright © 2025 Copyright, Monash University
//
// *********************************


module vga_controller (
	input [11:0] VGA_DATA, // 12-bit RGB: [11:8]=R, [7:4]=G, [3:0]=B
	input VGA_CLK,
	
	output [18:0] VGA_ADDR, // Address into video memory
	output [3:0] VGA_R,
	output [3:0] VGA_G,
	output [3:0] VGA_B,
	output VGA_HS,
	output VGA_VS
);

	// VGA Timing Parameters
	parameter H_SYNC  = 96;
	parameter H_BACK  = 48;
	parameter H_FRONT = 16;
	parameter H_CYCLE = 800;
	
	parameter V_SYNC  = 2;
	parameter V_BACK  = 33;
	parameter V_FRONT = 10;
	parameter V_CYCLE = 525;
	
	reg [18:0] H_ADDR, V_ADDR;

	// Horizontal Counter
	always @(posedge VGA_CLK) begin
		if (H_ADDR < H_CYCLE)
			H_ADDR <= H_ADDR + 1;
		else
			H_ADDR <= 0;
	end

	// Vertical Counter
	always @(posedge VGA_CLK) begin
		if (H_ADDR == H_CYCLE) begin
			if (V_ADDR < V_CYCLE)
				V_ADDR <= V_ADDR + 1;
			else
				V_ADDR <= 0;
		end
	end

	assign VGA_HS = (H_ADDR < H_SYNC) ? 1 : 0; 
	assign VGA_VS = (V_ADDR < V_SYNC) ? 1 : 0; 

	wire pixelValid;
	assign pixelValid = (H_ADDR >= 144 && H_ADDR < 784) && (V_ADDR >= 35 && V_ADDR < 515);

	// Assign RGB components from VGA_DATA when valid
	assign VGA_R = pixelValid ? VGA_DATA[11:8] : 4'd0;
	assign VGA_G = pixelValid ? VGA_DATA[7:4]  : 4'd0;
	assign VGA_B = pixelValid ? VGA_DATA[3:0]  : 4'd0;

	wire [18:0] H_MEM_ADDR, V_MEM_ADDR;	
	
	assign H_MEM_ADDR = H_ADDR - 144;
	assign V_MEM_ADDR = V_ADDR - 35;

	// Scale to 320x240 resolution
	assign VGA_ADDR = ((H_MEM_ADDR >> 1) + ((V_MEM_ADDR >> 1) * 320));
					
endmodule
