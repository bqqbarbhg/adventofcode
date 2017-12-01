#!/usr/bin/env python
import sys
import re

RE_WS = re.compile(r'\s+')
RE_LINE = re.compile(r'(\w+):capacity(-?\d+),durability(-?\d+),flavor(-?\d+),texture(-?\d+),calories(\d+)')
def parse_qualities(line):
	match = RE_LINE.match(RE_WS.sub('', line))
	return tuple(int(q) for q in match.groups()[1:])

qualities = [parse_qualities(line) for line in sys.stdin]

def mix(amount_left, base, index):
	if not index:
		value = tuple(a + b * amount_left for a,b in zip(base, qualities[index])) 
		if value[-1] != 500: return 0
		return reduce(lambda x,y: x*max(y,0), value[:-1], 1)
	return max(mix(amount_left - amount,
		tuple(a + b * amount for a,b in zip(base, qualities[index])),
		index - 1)
		for amount in range(amount_left + 1))

print mix(100, (0, 0, 0, 0, 0), len(qualities) - 1)
