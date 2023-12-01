module sine_wave_generator (
    input wire clk,                   // 125 MHz clock input
    input wire [15:0] freq_control,   // 16-bit frequency control input
    output wire [13:0] sine_out       // 14-bit sine wave output
);

	reg [31:0] phase_accumulator = 0; // Adjust size as needed for precision

	always @(posedge clk) begin
		 phase_accumulator <= phase_accumulator + freq_control;
	end

	reg [13:0] sine_lut[0:1023]; 
	initial $readmemh ("sine_wave_lut_values.txt", sine_lut);

	//assign sine_out = sine_lut[phase_accumulator[31:22]]; // fMax = 1.9kHz
	//assign sine_out = sine_lut[phase_accumulator[30:21]]; // fMax = 3.8kHz
	assign sine_out = sine_lut[phase_accumulator[17:8]]; // fMax = 31.25MHz fMin = 476Hz

endmodule