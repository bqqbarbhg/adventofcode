from collections import defaultdict, Counter
from itertools import chain
from functools import reduce
import sys
import re

dirs = dict(e=(1,0), w=(-1,0), se=(0,1), nw=(0,-1), sw=(-1,1), ne=(1,-1))
dirs_re = re.compile("|".join(re.escape(d) for d in dirs))
ichain = chain.from_iterable

def add(a, b):
    return (a[0]+b[0], a[1]+b[1])

def parse(line):
    return (dirs[m.group()] for m in dirs_re.finditer(line))

def black_tiles(tiles):
    return (c for c,t in tiles.items() if t)

def neighbors(tile):
    return (add(tile,d) for d in dirs.values())

def step_mut(tiles):
    nbcount = Counter(ichain(neighbors(t) for t in black_tiles(tiles)))
    for t in set(chain(nbcount, tiles)):
        alive, nbs = tiles[t], nbcount[t]
        if (nbs == 0 or nbs > 2 if alive else nbs == 2):
            tiles[t] ^= 1

tiles = defaultdict(int)
for line in (l.strip() for l in sys.stdin if l.strip()):
    tiles[reduce(add, parse(line), (0,0))] ^= 1
for n in range(int(sys.argv[1])):
    step_mut(tiles)
print(sum(tiles.values()))
