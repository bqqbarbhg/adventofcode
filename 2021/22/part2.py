import sys
import re
from math import inf, isinf

boxes = []

for line in sys.stdin:
    m = re.match(r"\s*(on|off)\s+x=(-?\d+)..(-?\d+),\s*y=(-?\d+)..(-?\d+),\s*z=(-?\d+)..(-?\d+)\s*", line)
    if not m: continue
    state,x0,x1,y0,y1,z0,z1 = m.groups()
    boxes.append(((int(x0),int(x1)+1), (int(y0),int(y1)+1), (int(z0),int(z1)+1), state == "on"))

def add_interval(ranges, path):
    if len(path) == 1: return path[0]
    pmin, pmax = path[0]
    result = []
    for omin, omax, odata in ranges:
        imin = min(max(pmin, omin), omax)
        imax = min(max(pmax, omin), omax)
        if imin > omin: result.append((omin, imin, odata))
        if imin < imax: result.append((imin, imax, add_interval(odata, path[1:])))
        if imax < omax: result.append((imax, omax, odata))
    return result

def count_set(ranges):
    if isinstance(ranges, bool): return ranges
    total = 0
    for rmin, rmax, rdata in ranges:
        num = rmax - rmin
        if not isinf(num):
            total += num * count_set(rdata)
    return total

grid = [(-inf, inf, [(-inf, inf, [(-inf, inf, False)])])]
for box in boxes:
    grid = add_interval(grid, box)
print(count_set(grid))
