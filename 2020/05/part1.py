#!/usr/bin/env python3
import sys

tab = str.maketrans("FLBR", "0011")
def seat_num(code):
    return int(code.translate(tab), base=2)

print(max(seat_num(line) for line in sys.stdin if line.strip()))
