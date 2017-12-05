#!/usr/bin/env python3
import sys

def uint(i):
    return i + 0x10000 if i < 0 else i

lines = list(sys.stdin)

print('v2.0 raw')
print('%04x' % uint(len(lines)))
print('\n'.join('%04x' % uint(int(line)) for line in lines))
