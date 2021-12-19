from collections import namedtuple
from collections import Counter
import itertools
import re
import sys

Scanner = namedtuple("Scanner", "index beacons")

def add(a, b): return (a[0]+b[0], a[1]+b[1], a[2]+b[2])
def sub(a, b): return (a[0]-b[0], a[1]-b[1], a[2]-b[2])
def mul(a, b): return (a[0]*b, a[1]*b, a[2]*b)
def mad(a, b, c): return (a[0]*b+c[0], a[1]*b+c[1], a[2]*b+c[2])
def transform(basis, a):
    return mad(basis[0], a[0], mad(basis[1], a[1], mul(basis[2], a[2])))

def make_orientations():
    sin = [0, 1, 0, -1]
    for axis in range(3):
        for sign in (-1, 1):
            for rotation in range(4):
                dx = sin[rotation]
                dy = sin[(rotation + 1) % 4]
                x, y, z = [0, 0, 0], [0, 0, 0], [0, 0, 0]
                x[axis] = sign
                y[(axis+1)%3] = dx * sign
                y[(axis+2)%3] = dy * sign
                z[(axis+1)%3] = -dy
                z[(axis+2)%3] = dx
                yield (tuple(x), tuple(y), tuple(z))
orientations = list(make_orientations())

def match(a, b):
    for basis in orientations:
        beacons_b = [transform(basis, bc) for bc in b.beacons]
        offsets = Counter(sub(ac, bc) for ac,bc in itertools.product(a.beacons, beacons_b))
        for offset, num in offsets.most_common():
            if num < 12: break
            if len(set(a.beacons) & set(add(bc, offset) for bc in beacons_b)) >= 12:
                return basis, offset

beacons = []
scanners = []
for line in sys.stdin:
    m = re.match(r"\s*---\s*scanner\s*(\d+)\s*--\s*", line)
    if m:
        if beacons:
            scanners.append(Scanner(len(scanners), beacons))
            beacons = []
        continue
    m = re.match(r"\s*(-?\d+)\s*,\s*(-?\d+)\s*,\s*(-?\d+)\s*", line)
    if m:
        beacons.append(tuple(int(c) for c in m.groups()))
if beacons:
    scanners.append(Scanner(len(scanners), beacons))

scanners_known = [(scanners[0], ((1,0,0),(0,1,0),(0,0,1)), (0,0,0))]
scanners_left = scanners[1:]

def match_next_scanner():
    for ix, dst in enumerate(scanners_left):
        for src, basis, offset in scanners_known:
            m = match(src, dst)
            if not m: continue
            m_basis, m_offset = m
            s_basis = (
                transform(basis, m_basis[0]),
                transform(basis, m_basis[1]),
                transform(basis, m_basis[2]),
            )
            s_offset = add(offset, transform(basis, m_offset))
            scanners_known.append((dst, s_basis, s_offset))
            del scanners_left[ix]
            return

while scanners_left:
    match_next_scanner()

def all_beacons():
    for scanner, basis, offset in scanners_known:
        for beacon in scanner.beacons:
            yield add(offset, transform(basis, beacon))

print(len(set(all_beacons())))
