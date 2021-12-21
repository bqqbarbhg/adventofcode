import sys
from collections import Counter
from itertools import product
from functools import cache

die_counts = Counter(sum(v) for v in product(range(1,4), repeat=3))

@cache
def count_wins(pos_a, score_a, pos_b, score_b):
    if score_b >= 21: return (1, 0)
    if score_a >= 21: return (0, 1)
    total_a, total_b = 0, 0
    for inc, num in die_counts.items():
        pos_n = (pos_a + inc - 1) % 10 + 1
        score_n = score_a + pos_n
        wins_b, wins_a = count_wins(pos_b, score_b, pos_n, score_n)
        total_a += num * wins_a
        total_b += num * wins_b
    return total_a, total_b

print(max(count_wins(int(sys.argv[1]), 0, int(sys.argv[2]), 0)))
