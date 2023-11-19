module FX2 (inout [15:0] FX2_data,
						input FX2_AS,
						input FX2_DS,
						input FX2_nRDWR,
						
						output wire [15:0] r0
						);

reg [3:0] addr_reg;			// Internal address register			
reg [15:0] io_regs[0:15];	// 15 registers
reg [15:0] data_bus;       // Internal data bus

assign r0 = io_regs[0];

// Tri-state buffer control for bidirectional data/address bus
assign FX2_data = (!FX2_nRDWR && FX2_DS) ? data_bus : 16'bz;

initial begin
	io_regs[0] = 16'h1234;
	io_regs[1] = 16'h5678;
end

	always @(posedge FX2_AS) begin
		addr_reg <= FX2_data [3:0];
	end
	
	always @(posedge FX2_DS) begin
		
		// read cycle
		if (FX2_nRDWR == 0) begin
			data_bus <= io_regs[addr_reg];
		end
			
		// write cycle
		else begin
			io_regs[addr_reg] <= FX2_data;
		end
		
	end
	
						
endmodule						