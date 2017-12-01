#!/usr/bin/env python
import sys

def paper_for_measures(measures):
    edges = tuple(int(edge) for edge in measures.split('x'));
    faces = tuple(edges[a] * edges[b] for a,b in ((0,1),(0,2),(1,2)))
    return 2 * sum(faces) + min(faces)

print sum(paper_for_measures(line) for line in sys.stdin)
