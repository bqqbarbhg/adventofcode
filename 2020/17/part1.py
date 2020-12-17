from typing import NamedTuple, Set, Iterable
from collections import Counter
from itertools import product, chain
import sys

class P(NamedTuple):
    x: int
    y: int
    z: int
    def __add__(self, rhs: "P") -> "P":
        return P(self.x + rhs.x, self.y + rhs.y, self.z + rhs.z)

Map = Set[P]
nb_dirs = [P(*t) for t in product((-1, 0, +1), repeat=3) if t != (0,0,0)]

def read_points(lines: Iterable[str]) -> Iterable[P]:
    for y, l in enumerate(lines):
        yield from (P(x, y, 0) for x,c in enumerate(l.strip()) if c == "#")

def alive(nbs: int, self_alive: bool) -> bool:
    return 2 <= nbs <= 3 if self_alive else nbs == 3

def step(m: Map) -> Map:
    nbs = Counter(p + n for p,n in product(m, nb_dirs))
    return set(p for p,c in nbs.items() if alive(c, p in m))

m = set(read_points(sys.stdin))
steps = int(sys.argv[1])
for _ in range(steps):
    m = step(m)
print(len(m))
