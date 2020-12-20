import sys
from functools import reduce
from operator import mul
from itertools import takewhile, cycle, islice, chain, product
from collections import namedtuple, Counter

Tile = namedtuple("Tile", "id opts inner")

def tile_rotations(sides):
    yield from (tuple(islice(cycle(sides), n, n+4)) for n in range(4))

def tile_flips(sides):
    yield from tile_rotations(sides)
    yield from tile_rotations(tuple(sides[i][::-1] for i in (2,1,0,3)))

def read_tiles(lines):
    while True:
        header = next(lines, "")
        if not header: break
        id = int(header[5:-1])
        ts = list(takewhile(lambda x: x, lines))
        sides = (
            "".join(ts[0]),
            "".join(ts[y][-1] for y in range(len(ts))),
            "".join(ts[-1][::-1]),
            "".join(ts[y][0] for y in range(len(ts) - 1, -1, -1)),
        )

        inner = [l[1:-1] for l in ts[1:-1]]
        yield Tile(id, list(tile_flips(sides)), inner)

def all_sides(tile):
    for opt in tile.opts:
        yield from opt

def place_tiles(tiles):
    side_count = Counter(s for t in tiles for s in all_sides(t))
    tile_rank = (sum(side_count[s] for s in all_sides(t)) for t in tiles)
    tile_prio = list(t for _,t in sorted(zip(tile_rank, tiles)))

    places = { (0,0) }
    placements = { }
    constraints = [{}, {}, {}, {}]

    neighbors = [(0,1), (1,0), (0,-1), (-1,0)]

    def place(tile, p, opt):
        places.remove(p)
        placements[p] = (tile.id, opt)
        for i,nb in enumerate(neighbors):
            nbp = (p[0] + nb[0], p[1] + nb[1])
            if nbp in placements: continue
            constraints[(i+2)%4][nbp] = tile.opts[opt][i][::-1]
            places.add(nbp)
    
    def fits(tile, p, opt):
        for i in range(4):
            side = constraints[i].get(p)
            if not side: continue
            if tile.opts[opt][i] != side: return False
        return True

    def place_next():
        for p in places:
            for tile_ix, tile in enumerate(tile_prio):
                for opt in range(len(tile.opts)):
                    if not fits(tile, p, opt): continue
                    place(tile, p, opt)
                    del tile_prio[tile_ix]
                    return True
        return False

    while place_next():
        pass

    # If this greedy algorithm doesn't work we need something smarter..
    assert not tile_prio

    return placements

if __name__ == "__main__":
    tiles = list(read_tiles(l.strip() for l in sys.stdin))
    placements = place_tiles(tiles)
    min_x, min_y = tuple(min(c[i] for c in placements) for i in range(2))
    max_x, max_y = tuple(max(c[i] for c in placements) for i in range(2))

    print(reduce(mul, (placements[x,y][0] for x,y in product((min_x, max_x), (min_y, max_y)))))
