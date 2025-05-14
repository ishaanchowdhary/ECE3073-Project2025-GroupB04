// Microsecond Timer Module

module usec_timer (
    input clk,          // FPGA system clock (e.g., 50 MHz)
    input reset,      // Active-low reset
    output reg [31:0] count_ext // 32-bit microsecond counter output
);
 // have a continous counter of micro seconds that have occured since the program has started

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
			clk_checker <= 1 + clk_checker;  // increment the counter until ya hit 50 (basically converts 50MHz to 1us)
		end
	end
end
		

endmodule