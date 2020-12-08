import sys
from nmigen import Module, Memory
from nmigen.back.pysim import Simulator
from shared import assemble, CPU

if __name__ == "__main__":
    code = assemble(sys.stdin)

    m = Module()

    memory = Memory(width=32, depth=len(code), init=code)
    m.submodules.cpu = cpu = CPU(memory)

    sim = Simulator(m)
    sim.add_clock(1e-6)

    def process():
        for n in range(10000):
            hlt = yield cpu.hlt
            if hlt: break
            yield # step CPU
        else:
            print("Did not halt!")
            return
        acc = yield cpu.acc
        print(acc)

    sim.add_sync_process(process)
    sim.run()
