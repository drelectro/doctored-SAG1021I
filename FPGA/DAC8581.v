module DAC8581 (
    input wire clk,                // System clock
    input wire reset,              // Reset signal
    input wire [15:0] dac_data,    // New data input for updating the DAC
    input wire load_new_data,      // Trigger to load new data
    output reg SCLK,               // Serial Clock for DAC
    output reg DIN,                // Serial Data Input for DAC
    output reg CS                  // Chip Select for DAC
);

// SPI interface parameters
parameter SCLK_FREQ = 1_000_000; // Adjust as per your system clock
parameter CLOCK_DIV = 25_000_000 / (2 * SCLK_FREQ); // Adjust for your FPGA clock
integer counter = 0;
reg [15:0] data_to_send = 0; // Default initialization data
reg [4:0] bit_index = 0; // DAC8581 is 16-bit

// State machine for SPI
reg [1:0] state = 0;
parameter IDLE = 0, LOAD = 1, SEND_BIT = 2, WAIT = 3;

always @(posedge clk) begin
    if (reset) begin
        // Reset logic
        SCLK <= 0;
        DIN <= 0;
        CS <= 1;
        state <= IDLE;
        bit_index <= 0;
        data_to_send <= 0; // Initial value
    end else begin
        case (state)
            IDLE: begin
                if (load_new_data) begin
                    data_to_send <= dac_data; // Load new data
                    CS <= 0; // Activate DAC
                    state <= SEND_BIT;
                end
            end
            SEND_BIT: begin
                if (counter < CLOCK_DIV) begin
                    counter <= counter + 1;
                end else begin
                    counter <= 0;
                    SCLK <= ~SCLK;
                    if (SCLK == 1) begin
                        DIN <= data_to_send[15 - bit_index];
                        bit_index <= bit_index + 1;
                        if (bit_index == 15) begin
                            state <= WAIT;
                            CS <= 1; // Deactivate DAC
                        end
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
 * U11 74HC405 DAC8581 output selector driver
 *
 */

module DAC8581_output_selector(output en, 
										output s0,
										output s1,
										output s2);
										
	assign en = 0;
	assign s0 = 1;
	assign s1 = 0;
	assign s2 = 0;
										
endmodule