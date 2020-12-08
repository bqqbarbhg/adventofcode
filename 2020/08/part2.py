import sys
from nmigen import Module, Memory
from nmigen.back.pysim import Simulator
from shared import assemble, CPU, Op

def run_sim(code):
    m = Module()

    memory = Memory(width=32, depth=len(code), init=code)
    m.submodules.cpu = cpu = CPU(memory)

    sim = Simulator(m)
    sim.add_clock(1e-6)

    reached_end = False

    def process():
        nonlocal reached_end
        for n in range(10000):
            hlt = yield cpu.hlt
            if hlt: break
            yield # step CPU
        else:
            return
        acc = yield cpu.acc
        hlt_arg = yield cpu.hlt_arg
        if hlt_arg == 1:
            reached_end = True
            print(acc)

    sim.add_sync_process(process)
    sim.run()

    return reached_end

if __name__ == "__main__":
    code = assemble(sys.stdin)
    code.append(Op.HLT | 1 << 4)  # hlt +1

    for n in range(len(code) - 1):
        op = code[n] & 0xffff_ffff
        opcode = Op(op & 0xf)
        arg = op >> 4
        if opcode == Op.NOP:
            do_patch = True
            patch_op = Op.JMP | arg << 4
        elif opcode == Op.JMP:
            do_patch = True
            patch_op = Op.NOP | arg << 4
        else:
            do_patch = False
        
        if do_patch:
            code[n] = patch_op
            done = run_sim(code)
            code[n] = op

            if done:
                break
