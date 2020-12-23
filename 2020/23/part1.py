import sys

cups = [int(v) for v in sys.argv[1]]
minc, maxc = min(cups), max(cups)

def below(c):
    return maxc if c == minc else c - 1

def step(cups):
    cur, pick = cups[0], cups[1:4]
    cups = cups[:1] + cups[4:]
    while True:
        cur = below(cur)
        try:
            dst = cups.index(cur)
            break
        except ValueError:
            pass
    cups = cups[:dst+1] + pick + cups[dst+1:]
    return cups[1:] + cups[0:1]

for n in range(int(sys.argv[2])):
    cups = step(cups)

ix = cups.index(1)
print("".join(str(v) for v in cups[ix+1:] + cups[:ix]))
