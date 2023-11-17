/*
Doctored SAG1021I

CLK4 = 25MHz CLK in
CLK6 = Unknown but connected


IOD2 = FX2_PA1 - U5_CS	(Boot flash CS)
DCLK = FX2_PE3 - U5-CLK 	
IOC1 = FX2_PA0 - U5-DQ0

 
IOA8 = Output LEDn 
IOD4 = Ready LEDn


IOB16 = FX2_CTL0
IOL13 = FX2_RDY0
IOL14 = FX2_RDY1

IOB10 = FX2_PB0
IOB11 = FX2_PB1
IOB12 = FX2_PB2
IOB13 = FX2_PB3
IOA11 = FX2_PB4
IOA12 = FX2_PB5
IOA13 = FX2_PB6
IOA14 = FX2_PB7

ION16 = FX2_PD0
IOP15 = FX2_PD1
IOR16 = FX2_PD2
IOP16 = FX2_PD3
ION14 = FX2_PD4
IOP14 = FX2_PD5
ION13 = FX2_PD6
ION12 = FX2_PD7


IOJ16 = FX2_PA2
IOJ15 = FX2_PA3
IOK16 = FX2_PA4
IOK15 = FX2_PA5
IOL16 = FX2_PA6
IOL15 = FX2_PA7

IOD15 = FX2_PC0
IOD16 = FX2_PC1
IOF14 = FX2_PC2
IOF15 = FX2_PC3
IOF16 = FX2_PC4


IOH2  = FX2_PE2
IOG16 = FX2_PE5
IOT15 = FX2_PE7


-- U6 MAIN DAC
IOL1  = DAC904.0  - CLK
IOT2  = DAC904.1  - D13
ION3  = DAC904.2  - D12
IOP3  = DAC904.3  - D11
IOR3  = DAC904.4  - D10
IOT3  = DAC904.5  - D9
IOR4  = DAC904.6  - D8
IOT4  = DAC904.7  - D7
ION5  = DAC904.8  - D6
IOR5  = DAC904.9  - D5
IOT5  = DAC904.10 - D4
ION6  = DAC904.11 - D3
IOP6  = DAC904.12 - D2
IOR6  = DAC904.13 - D1
IOT6  = DAC904.14 - D0

-- U10 DAC8581 
IOT10 = U10 - CSn
IOT11 = U10 - SDIN
ION9  = U10 - CLRn
IOR10 = U10 - SCLK

-- U11 DAC8581 output MUX
IOR13 = U11 - En
IOT13 = U11 - S0
IOT14 = U11 - S1
IOR14 = U11 - S2


IOT9  = U13 - En
IOR8  = U13 - S0
IOT8  = U13 - S1
IOR9  = U13 - S2


IOK9  = U15 - CS
IOL9  = U15 - SDATA
IOM9  = U15 - SCLK



IOP8  = K1 - 1
IOM7  = K1 - 2

ION2  = K2 - 1
IOR1  = K2 - 2

ION1  = K3 - 1
IOP1  = K3 - 2

IOT7  = K4 - 1
IOR7  = K4 - 2


*/

module ready_LED (input clk, 
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

module DAC8581_output_selector(output en, 
										output s0,
										output s1,
										output s2);
										
	assign en = 0;
	assign s0 = 1;
	assign s1 = 0;
	assign s2 = 0;
										
endmodule

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

module relay_driver(input state,
							output c1,
							output c2);
	
	assign c1 = ~state;
	assign c2 = state;
								
endmodule							
										


module SAG1021I (input clk, // 25MHz master clock

						output ready_led,
						//output reg output_led,
						
						output DAC8581_SCLK,
						output DAC8581_DIN,
						output DAC8581_CS,
						
						output U11_en,
						output U11_s0,
						output U11_s1,
						output U11_s2,
						
						output [13:0] dac904_data,
						output dac904_clk,
						
						output k1_1,
						output k1_2,
						output k2_1,
						output k2_2,
						output k3_1,
						output k3_2,
						output k4_1,
						output k4_2
						);
						
	ready_LED rdy(clk, 1, 1, ready_led);
	
	DAC8581 aux_dac(clk, 0, 16'h7000, 1, DAC8581_SCLK, DAC8581_DIN, DAC8581_CS);
	
	DAC8581_output_selector aux_dac_selector(U11_en, U11_s0, U11_s1, U11_s2);
	
	DAC904 main_dac(clk, dac904_clk, dac904_data);
	
	relay_driver K4(1, k4_1, k4_2); // Output Enable
	relay_driver K3(0, k3_1, k3_2); // High / Low level output
	//relay_driver K2(0, k2_1, k2_2); // ??
	//relay_driver K1(0, k1_1, k1_2); // ??	
	
endmodule	
