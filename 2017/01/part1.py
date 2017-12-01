#!/usr/bin/env python3
import sys
data = sys.stdin.read().strip()
print(sum(int(a) for a,b in zip(data, data[1:]+data[0]) if a==b))
