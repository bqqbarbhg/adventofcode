import sys
input = [int(l.strip()) for l in sys.stdin if l.strip()]
print(sum(a<b for a,b in zip(input, input[1:])))
