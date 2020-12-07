import sys
import functools
from shared import parse

specs = parse(sys.stdin.read())
target = tuple(sys.argv[1:])

@functools.lru_cache(maxsize=None)
def bag_count(bag):
    return sum((bag_count(b)+1)*c for b,c in specs[bag].items())

print(bag_count(target))
