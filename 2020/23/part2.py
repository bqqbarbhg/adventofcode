import sys

class Link:
    __slots__ = "p", "n"
    def __init__(self, p, n):
        self.p = p
        self.n = n

cups = [int(v) for v in sys.argv[1]]
minc, maxc = min(cups), max(cups)
addc = int(sys.argv[2])
if addc > maxc:
    cups += range(maxc+1, addc+1)
    maxc = addc
num = len(cups)

nbs = [None] * (maxc+1)
for n in range(num):
    a,b,c = cups[(n-1)%num], cups[n], cups[(n+1)%num]
    nbs[b] = Link(a, c)

def below(c):
    return maxc if c == minc else c - 1

def after(c, n=1):
    while n > 0:
        n -= 1
        c = nbs[c].n
    return c

def link(cp, cn):
    nbs[cp].n = cn
    nbs[cn].p = cp

def step(nbs, cur):
    takep = after(cur)
    taken = after(cur, 3)
    nextcur = after(taken)
    bad = (takep, after(takep), taken)

    dstp = below(cur)
    while dstp in bad:
        dstp = below(dstp)
    dstn = after(dstp)

    link(dstp, takep)
    link(taken, dstn)
    link(cur, nextcur)

    return nextcur

cur = cups[0]
for n in range(int(sys.argv[3])):
    cur = step(nbs, cur)
print(after(1) * after(1, 2))
