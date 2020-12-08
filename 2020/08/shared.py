from nmigen import *
from enum import IntEnum

class Op(IntEnum):
    NOP = 0
    ACC = 1
    JMP = 2
    HLT = 3

def assemble(lines):
    code = []
    for line in lines:
        parts = [l.strip() for l in line.split(" ") if l.strip()]
        if not parts: continue
        op = parts[0]
        arg = 0
        if len(parts) >= 2:
            arg = int(parts[1])

        if op == "nop":
            code.append(Op.NOP | arg << 4)
        elif op == "jmp":
            code.append(Op.JMP | arg << 4)
        elif op == "acc":
            code.append(Op.ACC | arg << 4)
        elif op == "hlt":
            code.append(Op.HLT | arg << 4)
    return code

class CPU(Elaboratable):
    def __init__(self, memory):
        self.memory = memory
        self.pc = Signal(16)
        self.acc = Signal(32, reset=0)
        self.pc_next = Signal(16)
        self.op_ready = Signal()
        self.op = Signal(32)
        self.opcode = Signal(Op)
        self.arg = Signal(32)
        self.hlt = Signal()
        self.hlt_arg = Signal(32)

    def elaborate(self, platform):
        m = Module()

        m.submodules.rd_port = rd_port = self.memory.read_port()
        m.submodules.wr_port = wr_port = self.memory.write_port()

        # Stay in place by default
        m.d.comb += self.pc_next.eq(self.pc)
        m.d.sync += self.pc.eq(self.pc_next)

        # Fetch instructions if not halted
        with m.If(~self.hlt):
            m.d.comb += rd_port.addr.eq(self.pc_next)
            m.d.sync += self.op_ready.eq(1)

        # If there's an operation ready read it and advance by default
        with m.If(self.op_ready):
            m.d.comb += self.op.eq(rd_port.data)
            m.d.comb += self.pc_next.eq(self.pc + 1)

        # Decode opcode and argument
        m.d.comb += [
            self.opcode.eq(self.op[0:4]),
            self.arg.eq(Cat(self.op[4:], Repl(self.op[-1], 4))),
        ]

        # Execute operations
        with m.Switch(self.opcode):
            with m.Case(Op.NOP):
                pass # nop
            with m.Case(Op.ACC):
                m.d.sync += self.acc.eq(self.acc + self.arg)
            with m.Case(Op.JMP):
                m.d.comb += self.pc_next.eq(self.pc + self.arg)
            with m.Case(Op.HLT):
                m.d.sync += self.hlt.eq(1)
                m.d.sync += self.hlt_arg.eq(self.arg)
                m.d.sync += self.op_ready.eq(0)
                m.d.comb += self.pc_next.eq(self.pc)
        
        # Clobber executed instructions with HLT
        with m.If(self.op_ready):
            m.d.comb += [
                wr_port.data.eq(Op.HLT | 0 << 4),
                wr_port.addr.eq(self.pc),
                wr_port.en.eq(1),
            ]

        return m
