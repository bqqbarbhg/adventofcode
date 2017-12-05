#!/usr/bin/env python3
import sys
import math

def f(n):
    return 1 + 4*n*(n + 1)

def fi(n):
    return math.ceil((-1 + math.sqrt(n)) / 2.0)

def distance(n):
    r = fi(n)
    return r + abs((n - f(r - 1)) % (2 * r) - r)

print(distance(int(sys.argv[1])))

