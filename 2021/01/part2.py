import sys
input = [int(l.strip()) for l in sys.stdin if l.strip()]
sums = [a+b+c for a,b,c in zip(input, input[1:], input[2:])]
print(sum(a<b for a,b in zip(sums, sums[1:])))
