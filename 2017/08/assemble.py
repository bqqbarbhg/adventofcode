#!/usr/bin/env python3
import sys
import re

registers = { }
dst_op = { 'inc': 0b0, 'dec': 0b1 }
cmp_op = {
    '==': 0b000, '!=': 0b100,
    '<' : 0b001, '>=': 0b101,
    '>' : 0b010, '<=': 0b110,
}

def register(name):
    if name in registers:
        return registers[name]
    else:
        num = len(registers)
        registers[name] = num
        return num

# 0: [cmp_not:1 cmp_op:2 dst_op:1 cmp_reg:6 dst_reg:6]
# 1: [cmp_imm:16]
# 2: [dst_imm:16]

def u16(num):
    if num < 0:
        return 0x10000 + num
    else:
        return num

def assemble(lines):
    LINE = re.compile(r'^\s*(?P<dr>\w+)\s*(?P<do>inc|dec)\s*(?P<di>-?\d+)\s*if\s*(?P<cr>\w+)\s*(?P<co>==|!=|<|>=|>|<=)\s*(?P<ci>-?\d+)\s*$')
    for l in lines:
        m = LINE.match(l)
        if not m: continue

        dr, cr = register(m.group('dr')), register(m.group('cr'))
        di, ci = int(m.group('di')), int(m.group('ci'))
        do, co = dst_op[m.group('do')], cmp_op[m.group('co')]
        
        yield co << 13 | do << 12 | cr << 6 | dr
        yield u16(ci)
        yield u16(di)

for inst in assemble(sys.stdin):
    print('%04x' % inst)
for hlt in range(3):
    print('ffff')

if len(registers) >= 64:
    print('Error: Too many registers!', file=sys.stderr)

