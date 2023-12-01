module function_generator (
    input wire clk,                 // 125 MHz clock input
    input wire [15:0] mode,   		// mode control input
	 input wire [15:0] amplitude,   	// amplitute input
	 input wire [15:0] offset,   		// offset input
	 input wire [31:0] conf_1_reg, 	// configuration input 1
	 input wire [31:0] conf_2_reg, 	// configuration input 2
	 input wire [31:0] conf_3_reg, 	// configuration input 3
	 
    output reg [13:0] wave_out     // 14-bit waveform output
);

integer cnt1 = 0;
integer cnt2 = 0;	
reg phase = 0;

wire [13:0] sine_data;
sine_wave_generator gen(clk, conf_1_reg[15:0], sine_data);

always @(posedge clk) begin
	case (mode)
		// Zero output
		0 : wave_out <= 0; 
		
		// DC output
		1 : wave_out <= conf_1_reg[13:0];  
		
		// Sine output
		2 : wave_out <= sine_data;	
		
		// Pulse Output
		3 : begin				
			if (phase == 0) begin
				wave_out <= conf_3_reg; // High
				if (cnt1 < conf_1_reg) begin
						cnt1 <= cnt1 + 1;
				end
				else begin
					cnt1 = 0;
					phase = 1;
				end 
			end
			
			else begin // phase 1
				wave_out <= 0;	// Low
				if (cnt2 < conf_2_reg) begin
						cnt2 <= cnt2 + 1;
				end
				else begin
					cnt2 = 0;
					phase = 0;
				end 
			end                    
		end
		
		default : wave_out <= 0;
		
	endcase
end

endmodule