module FX2 (inout [15:0] FX2_data,
						input FX2_AS,
						input FX2_DS,
						input FX2_nRDWR,
						
						output wire [15:0] control_reg, 		// Control register
						output wire [15:0] mode_reg, 			// Mode register
						output wire [15:0] amplitude_reg,	// Amplitude register
						output wire [15:0] offset_reg,		// Offset register
						output wire [31:0] conf_1_reg, 		// Configuration register 1 (frequency, High count, ...)
						output wire [31:0] conf_2_reg, 		// Configuration register 2 (Low count, ...)
						output wire [31:0] conf_3_reg, 		// Configuration register 2 (Cycle count, ...)
						
						
						
						input wire [15:0] adc_reg
						);

reg [4:0] addr_reg;			// Internal address register			
reg [15:0] io_regs[0:15];	// 15 registers
reg [15:0] data_bus;       // Internal data bus

assign control_reg = io_regs[0];
assign mode_reg = io_regs[1];
assign amplitude_reg = io_regs[2];
assign offset_reg = io_regs[3];
assign conf_1_reg = {io_regs[5], io_regs[4]};
assign conf_2_reg = {io_regs[7], io_regs[6]};
assign conf_3_reg = {io_regs[9], io_regs[8]};

initial begin
	io_regs[0] = 0;
	io_regs[1] = 0;
	io_regs[1] = 16'h6700;
end

// Tri-state buffer control for bidirectional data/address bus
assign FX2_data = (!FX2_nRDWR && FX2_DS) ? data_bus : 16'bz;

	// On Address Strobe
	always @(posedge FX2_AS) begin
		addr_reg <= FX2_data [4:0];
	end
	
	// On Data Strobe
	always @(posedge FX2_DS) begin
		
		// read cycle
		if (FX2_nRDWR == 0) begin
		
			// If address > 16 then this is a read only register
			// Read the value directly rather than from the register bank.
			// Presently only register 16 is defined.
			if (addr_reg == 16) begin
				data_bus <= adc_reg;
			end 
			else if (addr_reg > 16) begin
				data_bus <= 0;
			end 
			
			else begin 
				data_bus <= io_regs[addr_reg];
			end
		end
			
		// write cycle
		else begin
			// Ignore writes to read only registers
			if (addr_reg < 16) begin
				io_regs[addr_reg] <= FX2_data;
			end
		end
		
	end
	
						
endmodule						