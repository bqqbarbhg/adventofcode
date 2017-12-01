#!/usr/bin/env python
import primefac
import itertools
import operator
import sys

def factors(num):
    primedict = primefac.factorint(house)
    primelist = reduce(lambda x, y: x + [y[0]] * y[1], primedict.iteritems(), [])
    combos = reduce(operator.or_, (set(itertools.combinations(primelist, n)) for n in range(1, len(primelist) + 1)), set())
    return [reduce(operator.mul, c, 1) for c in combos] + [1]

def house_presents(house):
    return sum(f for f in factors(house) if house / f < 50) * 11

bound = int(sys.argv[1])
for house in itertools.count(1):
    num = house_presents(house)
    if num >= bound:
        print house
        break
