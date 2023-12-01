module ADC121S101 (
    input wire clk,                // System clock
    input wire reset,              // Reset signal
	 output reg [15:0] adc_data,    // New data input for updating the ADC
    output reg SCLK,               // Serial Clock for ADC
    input wire SDATA,              // Serial Data Output from ADC
    output reg CS                  // Chip Select for ADC
);

reg [15:0] data_in = 0;  // 
reg [4:0] bit_index = 0; // ADC121S101 is 12-bit + 3 leading bits

// State machine for SPI
reg [1:0] state = 0;
parameter IDLE = 0, LOAD = 1, READ_BIT = 2, WAIT = 3;

always @(posedge clk) begin
    if (reset) begin
        // Reset logic
        SCLK <= 0;
        CS <= 1;
        state <= IDLE;
        bit_index <= 0;
    end else begin
        case (state)
            IDLE: begin
				  CS <= 0; // Activate ADC
				  state <= READ_BIT;

            end
            READ_BIT: begin
				  SCLK <= ~SCLK;
				  if (SCLK == 1) begin
						data_in[15 - bit_index] <= SDATA;
						bit_index <= bit_index + 1;
						if (bit_index == 14) begin
							 state <= WAIT;
							 CS <= 1; // Deactivate ADC
							 adc_data <= data_in; // Load new data
						end
					end
            end
            WAIT: begin
                // Wait state or additional logic
                state <= IDLE; // Return to IDLE to accept new data
                bit_index <= 0;
            end
        endcase
    end
end

endmodule

/*
 * U13 74HC405 ADC121S101 input selector driver
 *
 */

module ADC121S101_input_selector(output en, 
										output s0,
										output s1,
										output s2);
										
	assign en = 0;
	assign s0 = 0;
	assign s1 = 0;
	assign s2 = 0;
										
endmodule