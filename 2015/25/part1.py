#!/usr/bin/env python

import itertools
import re
import sys

def gen(xx, yy):
    val = 20151125
    mul = 252533
    div = 33554393
    for n in itertools.count(1):
        y = n - 1
        x = 0
        for i in range(n):
            if x == xx and y == yy:
                return val

            val = val * mul % div
            y -= 1
            x += 1

row,col = (int(m.group(0)) for m in re.finditer(r'\d+', sys.stdin.read()))
print gen(col-1, row-1)
