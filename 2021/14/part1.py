import sys
from collections import Counter

lines = sys.stdin.readlines()

initial = "_" + lines[0].strip() + "_"
initial = [str(a + b) for a,b in zip(initial, initial[1:])]

transitions = {
    initial[0]: (initial[0],),
    initial[-1]: (initial[-1],),
}
for line in lines[1:]:
    line = line.strip()
    if not line: continue
    src, dst = line.split(" -> ")
    transitions[src] = ((src[0] + dst), (dst + src[1]))

steps = int(sys.argv[1])
prev_counts = Counter(initial)
for _ in range(steps):
    counts = Counter()
    for digraph, count in prev_counts.items():
        for t in transitions[digraph]:
            counts[t] += count
    prev_counts = counts

chars = Counter()
for digraph, count in prev_counts.items():
    for c in digraph:
        if c != "_":
            chars[c] += count
order = [(c,n//2) for c,n in chars.most_common()]
print(order[0][1] - order[-1][1])
