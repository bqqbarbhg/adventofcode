#!/usr/bin/env python3
import sys
import shared
from collections import Counter
from functools import lru_cache

@lru_cache(maxsize=None)
def total_weight(node):
    return sum(total_weight(c) for c in node.children) + node.weight

@lru_cache(maxsize=None)
def is_balanced(node):
    if len(node.children) < 2: return True
    first = total_weight(node.children[0])
    for other in node.children[1:]:
        if total_weight(other) != first:
            return False
    return True

def get_reference_weight(node):
    if len(node.children) < 3:
        return get_reference_weight(node.parent) - node.weight
    else:
        return Counter(total_weight(c) for c in node.children).most_common(1)[0][0]

nodes = shared.parse_nodes(sys.stdin)
for node in nodes.values():
    sb = is_balanced(node)
    cb = all(is_balanced(c) for c in node.children)
    if not sb and cb:
        for child in node.children:
            ref = get_reference_weight(node)
            tw = total_weight(child)
            if tw != ref:
                delta = ref - tw
                print(child.weight + delta)

