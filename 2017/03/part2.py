#!/usr/bin/env python3
import itertools
import math
import sys

def coords(size, ring = 1):
    if ring == size: return
    for i in range(-ring + 1, ring + 1):
        yield (ring, -i)
    for i in range(-ring + 1, ring + 1):
        yield (-i, -ring)
    for i in range(-ring + 1, ring + 1):
        yield (-ring, i)
    for i in range(-ring + 1, ring + 1):
        yield (i, ring)
    yield from coords(size, ring + 1)

def formatd(data):
    minx, miny = min(x for x,y in data.keys()), min(y for x,y in data.keys())
    maxx, maxy = max(x for x,y in data.keys()), max(y for x,y in data.keys())
    width = max(len(str(v)) for v in data.values())

    def showcol(row, col):
        val = data.get((col, row), '')
        return str(val).rjust(width)

    def showrow(row):
        return ' '.join(showcol(row, col) for col in range(minx, maxx + 1))

    return '\n'.join(showrow(row) for row in range(miny, maxy + 1))

def showd(data):
    print(formatd(data))

def make_basis(size):
    basis = { }
    basis[0,0] = 1

    prev = None
    for x,y in coords(size):
        s = 0
        s += basis.get((x-1,y), 0)
        s += basis.get((x+1,y), 0)
        s += basis.get((x,y-1), 0)
        s += basis.get((x,y+1), 0)
        s += basis.get((x-1,y-1), 0)
        s += basis.get((x-1,y+1), 0)
        s += basis.get((x+1,y-1), 0)
        s += basis.get((x+1,y+1), 0)

        basis[x,y] = s
        prev = (x,y)

    return basis

def find_upper_bound(num):
    for size in itertools.count(2):
        basis = make_basis(size)
        value = min((v for v in basis.values() if v > num), default=0)
        if value: return value

print(find_upper_bound(int(sys.argv[1])))
