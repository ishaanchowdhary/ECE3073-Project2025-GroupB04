module blurHelperFinal(
	input [31:0] dataa,
	input [31:0] datab,
	output [31:0] result



);

    // Internal wires
    wire [7:0] p1, p2, p3, p4, p5, p6;
    wire [3:0] val1, val2, val3, val4, val5, val6, val8, val9;

    // Unpack input bytes
    assign p1 = dataa[23:16];
    assign p2 = dataa[15:8];
    assign p3 = dataa[7:0];

    assign p4 = datab[23:16];
    assign p5 = datab[15:8]; // p5 is not used for val7
    assign p6 = datab[7:0];

    // Extract nibbles
    assign val1 = p1[3:0];
    assign val2 = p2[7:4];
    assign val3 = p2[3:0];

    assign val4 = p3[3:0];
    assign val5 = p4[7:4];
    assign val6 = p4[3:0];

	 assign val7 = p5[3:0];
    assign val8 = p6[7:4];
    assign val9 = p6[3:0];

    assign result = (val1 + val2 + val3 + val4 + val5 + val6 + val8 + val9)*57>>9;
	
	


endmodule







