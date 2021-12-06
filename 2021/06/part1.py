import sys
from collections import Counter

def simulate_fish(t):
    assert t >= 0
    return [6, 8] if t == 0 else [t - 1]

def simulate_all(counts):
    result = Counter()
    for t,n in counts.items():
        for u in simulate_fish(t):
            result[u] += n
    return result

counts = Counter(int(s.strip()) for s in sys.stdin.read().split(","))
for _ in range(int(sys.argv[1])):
    counts = simulate_all(counts)
print(sum(counts.values()))
