from typing import Set, Iterable, Tuple
from collections import Counter
from itertools import product, chain
import sys

P = Tuple[int, ...]
Map = Set[P]

def add(p: P, q: P) -> P:
    return tuple(a+b for a,b in zip(p, q))

def read_points(lines: Iterable[str], dim: int) -> Iterable[P]:
    zeros = [0] * (dim - 2)
    for y, l in enumerate(lines):
        yield from ((x, y, *zeros) for x,c in enumerate(l.strip()) if c == "#")

def alive(nbs: int, self_alive: bool) -> bool:
    return 2 <= nbs <= 3 if self_alive else nbs == 3

def step(m: Map, dim: int) -> Map:
    nb_dirs = [t for t in product((-1, 0, +1), repeat=dim) if any(a for a in t)]
    nbs = Counter(add(p, n) for p,n in product(m, nb_dirs))
    return set(p for p,c in nbs.items() if alive(c, p in m))

dim, steps = int(sys.argv[1]), int(sys.argv[2])
m = set(read_points(sys.stdin, dim))
for _ in range(steps):
    m = step(m, dim)
print(len(m))
