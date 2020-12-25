import sys
from itertools import count

mul, mod = 7, 20201227
public_keys = [int(l.strip()) for l in sys.stdin if l.strip()]
loop_size = next(n for n in count(1) if pow(mul, n, mod) == public_keys[0])
print(pow(public_keys[1], loop_size, mod))
