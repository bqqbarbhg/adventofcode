import part1
import sys
from itertools import product

monster = """
..................#.
#....##....##....###
.#..#..#..#..#..#...
""".strip().split("\n")

def rotate(inner, opt):
    w, h = len(inner[0]), len(inner)
    if opt == 0:
        return inner
    elif opt == 1:
        return list("".join(inner[y][w - x - 1] for y in range(h)) for x in range(w))
    elif opt == 2:
        return list(l[::-1] for l in inner[::-1])
    elif opt == 3:
        return list("".join(inner[h - y - 1][x] for y in range(h)) for x in range(w))

def flip(inner, opt):
    if opt < 4:
        return rotate(inner, opt)
    else:
        return rotate(inner[::-1], opt - 4)

def rows(inners):
    min_x, min_y = tuple(min(c[i] for c in inners) for i in range(2))
    max_x, max_y = tuple(max(c[i] for c in inners) for i in range(2))
    for y in range(max_y, min_y - 1, -1):
        chunk = zip(*(inners[x,y] for x in range(min_x, max_x + 1)))
        yield from ("".join(lines) for lines in chunk)

def get_taps(raster):
    for y,line in enumerate(raster):
        for x,ch in enumerate(line):
            if ch == "#": yield (x, y)

def match(target, taps, p):
    w, h = len(target[0]), len(target)
    for t in taps:
        x, y = p[0] + t[0], p[1] + t[1]
        if not (0 <= x < w and 0 <= y < h): return False
        if target[y][x] != "#": return False
    return True

def matches(target, taps):
    w, h = len(target[0]), len(target)
    for p in product(range(w), range(h)):
        if match(target, taps, p): yield p

def find_monsters(target):
    for opt in range(8):
        taps = list(get_taps(flip(monster, opt)))
        for p in matches(target, taps):
            yield (p, opt)

def ilen(iter):
    return sum(1 for _ in iter)

if __name__ == "__main__":
    tiles = list(part1.read_tiles(l.strip() for l in sys.stdin))
    placements = part1.place_tiles(tiles)
    tile_map = { t.id: t for t in tiles }
    inners = { p: flip(tile_map[t].inner, o) for p, (t,o) in placements.items() }
    target = list(flip(list(rows(inners)), 3))
    monsters = find_monsters(target)

    # Let's just hope they don't overlap...
    print(ilen(get_taps(target)) - ilen(monsters) * ilen(get_taps(monster)))
