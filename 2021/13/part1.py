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

points = set(reflect(folds[0], p) for p in points)

print(len(points))

