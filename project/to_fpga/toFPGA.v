module toFPGA (CLOCK_50, KEY, LEDG);
	input CLOCK_50;
	input [3:0] KEY;
	output [8:0] LEDG;
	
	arquitetura U0 (.clk_clk(CLOCK_50), .resetn_reset_n(KEY[0]));
	
	assign LEDG[0] = KEY[0];

endmodule
