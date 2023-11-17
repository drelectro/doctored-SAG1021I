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


module relay_driver(input state,
							output c1,
							output c2);
	
	assign c1 = ~state;
	assign c2 = state;
								
endmodule							
										


module SAG1021I (input clk, // 25MHz master clock

						output ready_led,
						output output_led,
						
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

	wire clk_125MHz;
	//wire clk_125MHz_lock;
	CLK_125MHz(0, clk, clk_125MHz);
	
	status_LED rdy_led(clk, 0, 0, ready_led); // Off
	status_LED op_led(clk, 0, 1, output_led); // Flash
	
	DAC8581 aux_dac(clk, 0, 16'h7000, 1, DAC8581_SCLK, DAC8581_DIN, DAC8581_CS);
	
	DAC8581_output_selector aux_dac_selector(U11_en, U11_s0, U11_s1, U11_s2);
	
	
	
	DAC904 main_dac(clk_125MHz, dac904_clk, dac904_data);
	
	relay_driver K4(1, k4_1, k4_2); // Output Enable
	relay_driver K3(0, k3_1, k3_2); // High / Low level output
	//relay_driver K2(0, k2_1, k2_2); // ??
	//relay_driver K1(0, k1_1, k1_2); // ??	
	
endmodule	
