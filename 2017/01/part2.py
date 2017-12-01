#!/usr/bin/env python3
import sys
data = sys.stdin.read().strip()
mid = len(data) // 2
print(sum(int(a) for a,b in zip(data, data[mid:]+data[:mid]) if a==b))
