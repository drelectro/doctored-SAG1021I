module DAC904 (input clk,
					output dac904_clk,
					output reg [13:0] dac904_data);

	wire [13:0] data; 
	//sine_wave_generator gen(clk, 60000, data); // 50.780 MHz
	//sine_wave_generator gen(clk, 1000, data); // 2.929 MHz
	//sine_wave_generator gen(clk, 2000, data); // 5,859 MHz
	sine_wave_generator gen(clk, 3000, data); // 8.789 MHz
	//sine_wave_generator gen(clk, 3300, data); // 27.831 MHz
	//sine_wave_generator gen(clk, 4000, data); // 11.718 MHz
	
	always @(negedge clk) begin
		//	dac904_data <= dac904_data +1;
		//dac904_data <= 14'h3FFF; // 680mV
		//dac904_data <= 14'h0; // -370mV
		
		
		dac904_data <= data;
	end
	
	
	
	assign dac904_clk = clk;
					
endmodule	