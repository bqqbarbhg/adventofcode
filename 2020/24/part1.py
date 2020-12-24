from collections import defaultdict
from functools import reduce
import sys
import re

dirs = dict(e=(1,0), w=(-1,0), se=(0,1), nw=(0,-1), sw=(-1,1), ne=(1,-1))
dirs_re = re.compile("|".join(re.escape(d) for d in dirs))

def add(a, b):
    return (a[0]+b[0], a[1]+b[1])

def parse(line):
    return (dirs[m.group()] for m in dirs_re.finditer(line))

tiles = defaultdict(int)
for line in (l.strip() for l in sys.stdin if l.strip()):
    tiles[reduce(add, parse(line), (0,0))] ^= 1
print(sum(tiles.values()))
