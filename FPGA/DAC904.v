module DAC904 (input clk,
					output dac904_clk,
					output reg [13:0] dac904_data);

	always @(negedge clk) begin
		dac904_data <= dac904_data +1;
		//dac904_data <= 14'h3FFF; // 680mV
		//dac904_data <= 14'h0; // -370mV
	end
	
	assign dac904_clk = clk;
					
endmodule	