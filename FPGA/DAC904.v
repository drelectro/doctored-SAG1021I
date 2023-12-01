module DAC904 (input clk,
					input [13:0] data,
					output dac904_clk,
					output reg [13:0] dac904_data
					);
	
	always @(negedge clk) begin		
		dac904_data <= data;
	end
	
	assign dac904_clk = clk;
					
endmodule	