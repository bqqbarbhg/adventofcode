import sys

def bits_to_int(bits):
    return sum(b<<i for i,b in enumerate(bits))

lines = [l.strip() for l in sys.stdin if l.strip()]
width, depth = len(lines[0]), len(lines)
counts = [sum(int(l[c]) for l in lines) for c in range(width - 1, -1, -1)]
gamma = bits_to_int(c > depth/2 for c in counts)
epsilon = bits_to_int(c < depth/2 for c in counts)
print(gamma * epsilon)
