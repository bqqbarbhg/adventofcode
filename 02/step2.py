#!/usr/bin/env python
import sys
import operator

def ribbon_for_measures(measures):
    edges = tuple(int(edge) for edge in measures.split('x'));
    perimeters = tuple(2 * (edges[a] + edges[b]) for a,b in ((0,1),(0,2),(1,2)))
    return min(perimeters) + reduce(operator.mul, edges)

print sum(ribbon_for_measures(line) for line in sys.stdin)

