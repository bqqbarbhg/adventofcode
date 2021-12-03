import sys

def rating(lines, invert):
    bit = 0
    while len(lines) > 1:
        count = sum(int(l[bit]) for l in lines)
        target = (count >= len(lines)/2) ^ invert
        lines = [l for l in lines if int(l[bit]) == target]
        bit += 1
    return int(lines[0], base=2)

lines = [l.strip() for l in sys.stdin if l.strip()]
width = len(lines[0])
counts = [sum(int(l[c]) for l in lines) for c in range(width)]
oxygen = rating(lines, 0)
scrubber = rating(lines, 1)
print(oxygen * scrubber)
