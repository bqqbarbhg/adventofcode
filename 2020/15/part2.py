import sys

seed = [int(v) for v in sys.argv[1].split(",")]
pos = { v: (i, i) for i,v in enumerate(seed) }
last = seed[-1]

for n in range(len(seed), int(sys.argv[2])):
    cur, prev = pos.get(last, (0, 0))
    last = cur - prev
    pos[last] = (n, pos[last][0] if last in pos else n)

print(last)
