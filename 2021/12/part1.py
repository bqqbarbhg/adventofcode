import sys
from collections import defaultdict

graph = defaultdict(list)

for line in sys.stdin.readlines():
    line = line.strip()
    if not line: continue
    a, b = line.split("-")
    graph[a].append(b)
    graph[b].append(a)

def explore(node="start", seen=set()):
    if node == "end":
        yield [node]
    seen = seen | { node }
    for next in graph[node]:
        if next.islower() and next in seen:
            continue
        for path in explore(next, seen):
            yield [node, *path]

print(sum(1 for _ in explore()))
