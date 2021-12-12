import sys
from collections import defaultdict

graph = defaultdict(list)

for line in sys.stdin.readlines():
    line = line.strip()
    if not line: continue
    a, b = line.split("-")
    graph[a].append(b)
    graph[b].append(a)

def explore(node="start", seen={ "start" }, has_skip=True):
    if node == "end":
        yield (node,)
    for next in graph[node]:
        if next.islower() and next in seen:
            continue
        for path in explore(next, seen | { next }, has_skip):
            yield (node, *path)
        if has_skip and next.islower() and next not in ("start", "end"):
            for path in explore(next, seen, False):
                yield (node, *path)

print(len(set(explore())))

