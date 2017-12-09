
module imem(addr, data);
	input wire [15:0]  addr; // Selected memory address
	output wire [15:0] data; // Data of the memory cell
	assign data = memory[addr];

	reg [15:0] memory [0:65536];

	initial begin
		$readmemh("input.list", memory);
	end
endmodule

module decoder(bits, cmp_not, cmp_op, dst_op, cmp_reg, dst_reg);
	input wire [15:0] bits;    // Encoded bits for the opcode
	output wire       cmp_not; // Negate the comparson output
	output wire [1:0] cmp_op;  // Comparison operator
	output wire       dst_op;  // Destination operator
	output wire [5:0] cmp_reg; // Register to use for comparison
	output wire [5:0] dst_reg; // Register to use for operation
	assign { cmp_not, cmp_op, dst_op, cmp_reg, dst_reg } = bits;
endmodule

module comparator(a, b, op, neg, out);
	input wire signed [31:0] a;   // First operand
	input wire signed [31:0] b;   // Second operand
	input wire [1:0]  op;  // Operator
	input wire        neg; // Negate the result
	output wire       out; // Ouptut result

	reg cmp;
	assign out = cmp ^ neg;

	always @(op, a, b) begin
		case (op)
			2'd0: assign cmp = a == b;
			2'd1: assign cmp = a < b;
			2'd2: assign cmp = a > b;
		endcase
	end

endmodule

module cpu(clk, reset, mema, memd);
	input wire clk;          // Clock signal
	input wire reset;        // Reset signal
	output wire [15:0] mema; // Memory address
	input wire [15:0] memd;  // Memory data

	assign mema = pc;

	reg signed [31:0] registers [0:64];
	reg signed [31:0] cmp_reg;

	reg [15:0] pc;

	// Decoded opcode
	reg [15:0] opcode;
	wire       op_cmp_neg;
	wire [1:0] op_cmp_op;
	wire       op_dst_op;
	wire [5:0] op_cmp_reg;
	wire [5:0] op_dst_reg;

	reg signed [15:0] cmp_imm;
	wire signed [31:0] cmp_imm_wide;

	wire signed [31:0] dst_imm_wide;

	reg [1:0] phase;

	wire hlt;
	assign hlt = opcode == 16'hffff;

	decoder Decoder(opcode, op_cmp_neg, op_cmp_op, op_dst_op,
		op_cmp_reg, op_dst_reg);

	wire cmp_result;
	comparator Comparator(cmp_reg, cmp_imm_wide, op_cmp_op, op_cmp_neg,
		cmp_result);

	assign cmp_imm_wide = { {16{cmp_imm[15]}}, cmp_imm };
	assign dst_imm_wide = { {16{memd[15]}}, memd };

	integer i;
	always @reset begin
		if (reset) begin
			pc <= 0;
			phase <= 0;
			opcode <= 0;
			for (i = 0; i < 64; i = i + 1) begin
				registers[i] <= 0;
			end
		end
	end

	always @(posedge clk) begin
		if (!hlt) begin
			pc <= pc + 1;
			case (phase)
				2'd0: begin // Fetch and decode the opcode
					opcode <= memd;
					phase <= 2'd1;
				end
				2'd1: begin // Fetch compare imm and register
					cmp_imm <= memd;
					cmp_reg <= registers[op_cmp_reg];
					phase <= 2'd2;
				end
				2'd2: begin // Execute the instruction !
					if (cmp_result) begin
						case (op_dst_op)
							1'd0: registers[op_dst_reg] <= registers[op_dst_reg] + dst_imm_wide;
							1'd1: registers[op_dst_reg] <= registers[op_dst_reg] - dst_imm_wide;
						endcase
					end
					phase <= 2'd0;
				end
			endcase
		end
	end
		
endmodule

module test;

	reg reset = 0;
	initial begin
		# 1 reset = 1;
		# 2 reset = 0;

		# 100010 $stop;
	end

	reg clk = 0;
	always #5 clk = !clk;

	wire [15:0] mema, memd;
	imem Memory(mema, memd);
	cpu Cpu(clk, reset, mema, memd);

	// always #30 $display("P%d DR:%d CR:%d CMPRES:%d CO: %d, DO:%d R1:%d", Cpu.phase, Cpu.op_dst_reg, Cpu.op_cmp_reg, Cpu.cmp_result, Cpu.op_cmp_op, Cpu.op_dst_op, Cpu.registers[1]);

	integer signed i;
	integer signed max;
	initial #100000 begin
		max = 0;
		for (i = 0; i < 64; i = i + 1) begin
			if (Cpu.registers[i] > max) max = Cpu.registers[i];
		end
		$display("MAX: %d", max);
	end

endmodule

