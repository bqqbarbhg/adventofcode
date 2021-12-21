import itertools
import sys

die = itertools.cycle(x+1 for x in range(100))
def sums(it):
    while True:
        yield next(it) + next(it) + next(it)

def play(it, pos_a, pos_b):
    score_a, score_b = 0, 0
    for turn in itertools.count():
        pos_a = (pos_a + next(it) - 1) % 10 + 1
        score_a += pos_a
        if score_a >= 1000: return score_b * ((turn * 2 + 1) * 3)
        pos_b = (pos_b + next(it) - 1) % 10 + 1
        score_b += pos_b
        if score_b >= 1000: return score_a * ((turn * 2 + 2) * 3)

print(play(sums(die), int(sys.argv[1]), int(sys.argv[2])))
