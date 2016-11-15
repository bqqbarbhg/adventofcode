#!/usr/bin/env python

import sys
from collections import defaultdict

def replace(molecule, replacements):
    length = len(molecule)
    for start in range(length):
        for src,dst in replacements.items():
            if start + len(src) > length:
                continue
            if molecule[start:start+len(src)] == src:
                for d in dst:
                    yield molecule[:start] + d + molecule[start+len(src):]

replacements = defaultdict(list)

for line in (l.strip() for l in sys.stdin):
    if not line: break
    src, dst = tuple(line.split(' => '))
    replacements[src].append(dst)

molecule = next(sys.stdin).strip()

print len(set(replace(molecule, replacements)))
