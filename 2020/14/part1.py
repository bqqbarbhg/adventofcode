import sys
import re

ops = []

def op(rx):
    return lambda fn: ops.append((re.compile(rx), lambda s, m: fn(s, *m.groups())))

class State:
    def __init__(self):
        self.mask_x = 0
        self.mask_1 = 0
        self.memory = { }

@op(r"mask\s*=\s*([01X]+)")
def op_mask(s, mask):
    s.mask_x = int(mask.replace("1", "0").replace("X", "1"), 2)
    s.mask_1 = int(mask.replace("X", "0"), 2)

@op(r"mem\[(\d+)\]\s*=\s*(\d+)")
def op_store(s, addr, value):
    s.memory[addr] = int(value) & s.mask_x | s.mask_1

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
