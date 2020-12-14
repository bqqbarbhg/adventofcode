import sys
import re

ops = []

def op(rx):
    return lambda fn: ops.append((re.compile(rx), lambda s, m: fn(s, *m.groups())))

class State:
    def __init__(self):
        self.mask_x = 0
        self.mask_1 = 0
        self.mask_x_pop = 0
        self.memory = { }

def pdep(value, mask):
    result = 0
    while mask:
        prev = mask
        mask &= mask - 1
        result += (mask ^ prev) * (value & 1)
        value >>= 1
    return result

@op(r"mask\s*=\s*([01X]+)")
def op_mask(s, mask):
    s.mask_x = int(mask.replace("1", "0").replace("X", "1"), 2)
    s.mask_1 = int(mask.replace("X", "0"), 2)
    s.mask_x_pop = mask.count("X")

@op(r"mem\[(\d+)\]\s*=\s*(\d+)")
def op_store(s, addr, value):
    addr = int(addr) & ~s.mask_x | s.mask_1
    for n in range(1 << s.mask_x_pop):
        s.memory[addr | pdep(n, s.mask_x)] = int(value)

state = State()

for line in sys.stdin:
    line = line.strip()
    for regex, fn in ops:
        m = regex.match(line)
        if not m: continue
        fn(state, m)
        break
    else:
        raise ValueError("Syntax error: " + line)

print(sum(state.memory.values()))
