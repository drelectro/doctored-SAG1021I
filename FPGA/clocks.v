module clock_gen (input clk, 
						output reg clk_12_5MHz,
						output reg clk_100Hz);
						
	reg [18:0] counter;
	
	always@ (posedge clk)
	begin
		clk_12_5MHz <= counter[1];	// Divide by 2
			
		if (counter <= 250000)
			begin
				counter <= counter +1; 
			end
		
		else
			begin
				counter <= 0;
				clk_100Hz <= ~clk_100Hz;
			end
	end
	
						
endmodule
