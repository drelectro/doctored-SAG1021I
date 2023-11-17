module status_LED (input clk, 
						input state,
						input flash,
						output reg led);
						
	reg [31:0] counter;
	
	always@ (posedge clk)
	begin
		if (flash == 1)
		begin
			if (counter <= 2500000)
				begin
					counter <= counter +1; 
				end
			
			else
				begin
					counter <= 0;
					led <= ~led;
				end
		end
		else 
			begin
				led <= ~state;
			end
	end
	
						
endmodule