/*
module relay_driver(input clk,
						  input state,
							output reg c1,
							output reg c2);
							
	reg state_changed;
	reg new_state;
	
	always @(state) begin
		state_changed <= 1;
		new_state <= state;
	end
	
	always @(clk) begin
		if ((state_changed == 1) && (clk ==1)) begin
			c1 <= ~new_state;
			c2 <= new_state;
		end
		
		else if (clk == 0) begin
			state_changed <= 0;
			c1 <= 0;
			c2 <= 0;
		end 
	end
		
								
endmodule
*/

module relay_driver(input clk,
							input state,
							output c1,
							output c2);
	
	assign c1 = ~state;
	assign c2 = state;
								
endmodule