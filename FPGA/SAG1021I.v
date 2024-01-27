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

IOB10 = FX2_PB0 D0
IOB11 = FX2_PB1
IOB12 = FX2_PB2
IOB13 = FX2_PB3
IOA11 = FX2_PB4
IOA12 = FX2_PB5
IOA13 = FX2_PB6
IOA14 = FX2_PB7 D7

ION16 = FX2_PD0 D8
IOP15 = FX2_PD1
IOR16 = FX2_PD2
IOP16 = FX2_PD3
ION14 = FX2_PD4
IOP14 = FX2_PD5
ION13 = FX2_PD6
ION12 = FX2_PD7 D15


IOJ16 = FX2_PA2
IOJ15 = FX2_PA3
IOK16 = FX2_PA4
IOK15 = FX2_PA5
IOL16 = FX2_PA6
IOL15 = FX2_PA7

IOD15 = FX2_PC0 AS
IOD16 = FX2_PC1 DS
IOF14 = FX2_PC2 nRD/WR
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

 -- U15 ADC121S101
IOK9  = U15 - CS
IOL9  = U15 - SDATA
IOM9  = U15 - SCLK

-- U16 AUX I/O Driver

IOT12 = AUX_IN_ENn
IOR11 = AUX_IN
IOR12 = AUX_OUT_ENn
IOP11 = AUX_OUT


-- Relay drivers

IOP8  = K1 - 1
IOM7  = K1 - 2

ION2  = K2 - 1
IOR1  = K2 - 2

ION1  = K3 - 1
IOP1  = K3 - 2

IOT7  = K4 - 1
IOR7  = K4 - 2


*/

							
module SAG1021I (input clk_25MHz, // 25MHz master clock

						output ready_led,
						output output_led,
						
						output k1_1,
						output k1_2,
						output k2_1,
						output k2_2,
						output k3_1,
						output k3_2,
						output k4_1,
						output k4_2,
						
						output DAC8581_SCLK,
						output DAC8581_DIN,
						output DAC8581_CS,
						
						output U11_en,
						output U11_s0,
						output U11_s1,
						output U11_s2,
						
						output ADC121S101_SCLK,
						output ADC121S101_CS,
						input  ADC121S101_SDATA,
						
						output U13_en,
						output U13_s0,
						output U13_s1,
						output U13_s2,
						
						
						output AUX_OUT_ENn,
						output AUX_OUT,
						output AUX_IN_ENn,
						input  AUX_IN,
						
						output [13:0] dac904_data,
						output dac904_clk,
						
						inout [15:0] FX2_data,
						input FX2_AS,
						input FX2_DS,
						input FX2_nRDWR
						
						);

	// Set up clocks							
	wire clk_125MHz;
	CLK_125MHz clk_pll(0, clk_25MHz, clk_125MHz);
	
	wire clk_12_5MHz;
	wire clk_100Hz;
	clock_gen clk_div(clk_25MHz, clk_12_5MHz, clk_100Hz);
		
	//status_LED rdy_led(clk_25MHz, 0, 0, ready_led); // Off
	status_LED op_led(clk_100Hz, 0, 1, output_led); // Flash output LED
	
	wire [15:0] control_reg; 		// Control register
	wire [15:0] mode_reg; 			// Mode register
	wire [15:0] amplitude_reg;	   // Amplitude register
	wire [15:0] offset_reg;		   // Offset register
	wire [31:0] conf_1_reg; 		// Configuration register 1 (frequency, High count, ...)
	wire [31:0] conf_2_reg; 		// Configuration register 2 (Low count, ...)
	wire [31:0] conf_3_reg; 		// Configuration register 2 (Cycle count, ...)
	wire [15:0] aux_adc_reg;		// ADC Read register
	
	reg  [15:0] wf_mem_addr_i;		// Waveform memory address register (WR)
	wire [15:0] wf_mem_data_i;		// Waveform memory data register (WR)
	wire wf_mem_clk_i;				// Waveform memory clock (WR)
	
	FX2 fx2 (FX2_data, FX2_AS, FX2_DS, FX2_nRDWR, 
		control_reg, mode_reg, amplitude_reg, offset_reg,
		conf_1_reg, conf_2_reg, conf_3_reg,
		aux_adc_reg,
		wf_mem_data_i,wf_mem_clk_i);
		
	// On chip memory
	reg [13:0] wf_mem_addr_o;		// Waveform memory address register (RD)
	wire [13:0] wf_mem_data_o;		// Waveform memory data register (RD)
	
	
	always@ (posedge clk_125MHz)
	begin
		wf_mem_addr_o <= wf_mem_addr_o + 1;
	end
	
	memory mem(wf_mem_data_i[13:0], wf_mem_addr_o, clk_125MHz,
				  wf_mem_addr_i[14:0], wf_mem_clk_i, 1, wf_mem_data_o);
	
	//data,
	//rdaddress,
	//rdclock,
	//wraddress,
	//wrclock,
	//wren,
	//q);
	
	always@ (negedge wf_mem_clk_i)
	begin
		wf_mem_addr_i <= wf_mem_addr_i + 1;
	end
	
	// Aux IO
	
	assign AUX_IN_ENn = 1;
	assign AUX_OUT_ENn = 0;
	
	assign AUX_OUT = wf_mem_addr_i[1];
	
	
	DAC8581 aux_dac(clk_25MHz, 0, amplitude_reg, 1, DAC8581_SCLK, DAC8581_DIN, DAC8581_CS);
	DAC8581_output_selector aux_dac_selector(U11_en, U11_s0, U11_s1, U11_s2);
	
	ADC121S101 aux_adc(clk_12_5MHz, 0, aux_adc_reg, ADC121S101_SCLK, ADC121S101_SDATA, ADC121S101_CS);
	ADC121S101_input_selector aux_adc_selector(U13_en, U13_s0, U13_s1, U13_s2);	
	
	
	wire [13:0] fg_data; 
	//sine_wave_generator gen(clk_125MHz, conf_1_reg[15:0], waveform_data);
	function_generator gen(clk_125MHz, mode_reg, amplitude_reg, offset_reg, conf_1_reg, conf_2_reg, conf_3_reg, fg_data);
	
	wire [13:0] waveform_data;
	assign waveform_data = control_reg[15] ? wf_mem_data_o : fg_data;
	DAC904 main_dac(clk_125MHz, waveform_data, dac904_clk, dac904_data);
	
	status_LED rdy_led(clk_100Hz, control_reg[0], 0, ready_led); // 

	/* If we conenct relay K4 to control_reg[0] the ouput glitches at zero crossing, but it's fine as follows, why? */
	relay_driver K4(clk_100Hz, control_reg[1], k4_1, k4_2); // Output Enable
	relay_driver K3(clk_100Hz, control_reg[2], k3_1, k3_2); // Enable output amplifier (x ?)
	relay_driver K2(clk_100Hz, control_reg[3], k2_1, k2_2); // Enable output attenuator (/10 ?)
	relay_driver K1(clk_100Hz, control_reg[4], k1_1, k1_2); // Enable ouput LPF	
	
endmodule	
