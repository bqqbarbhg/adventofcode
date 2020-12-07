import sys
import functools
from shared import parse

specs = parse(sys.stdin.read())
target = tuple(sys.argv[1:])

@functools.lru_cache(maxsize=None)
def bag_contains(target, bag):
    return any(b == target or bag_contains(target, b) for b in specs[bag])

print(sum(bag_contains(target, b) for b in specs))
