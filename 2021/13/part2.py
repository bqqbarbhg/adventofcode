import sys
import re

def parse_points(lines):
    for line in lines:
        m = re.match(r"(\d+),(\d+)", line)
        if not m: continue
        yield int(m.group(1)), int(m.group(2))

def parse_folds(lines):
    for line in lines:
        m = re.match(r"fold along ([xy])=(\d+)", line)
        if not m: continue
        yield "xy".index(m.group(1)), int(m.group(2))

def reflect(fold, point):
    axis, pos = fold
    if point[axis] > pos:
        p = list(point)
        p[axis] = 2*pos - p[axis]
        return tuple(p)
    else:
        return point
    
points, folds = sys.stdin.read().split("\n\n")
points = set(parse_points(points.split("\n")))
folds = list(parse_folds(folds.split("\n")))

for fold in folds:
    points = set(reflect(fold, p) for p in points)

width = max(points, key=lambda p: p[0])[0]
height = max(points, key=lambda p: p[1])[1]
for y in range(height + 1):
    print("".join(" #"[(x,y) in points] for x in range(width + 1)))
