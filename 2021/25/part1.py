import numpy as np
import sys
import itertools

lines = [line.strip() for line in sys.stdin if line.strip()]

h = np.stack([np.array([c == ">" for c in line], dtype=bool) for line in lines])
v = np.stack([np.array([c == "v" for c in line], dtype=bool) for line in lines])

def step(h, v):
    mh = h & ~np.roll(h|v, -1, axis=1)
    h = (h & ~mh) | np.roll(mh, 1, axis=1)
    mv = v & ~np.roll(h|v, -1, axis=0)
    v = (v & ~mv) | np.roll(mv, 1, axis=0)
    return h, v

for n in itertools.count(start=1):
    hp, vp = step(h, v)
    if np.all(hp == h) and np.all(vp == v):
        print(n)
        break
    h, v = hp, vp
