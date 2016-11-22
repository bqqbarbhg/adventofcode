#!/usr/bin/env python

import sys
import itertools
import operator
import heapq
from collections import namedtuple

def partitions(weights):
    third = sum(weights) / 3
    for num in itertools.count(1):
        for c in itertools.combinations(weights, num):
            if sum(c) == third:
                center = set(c)
                sides = weights - center
                yield (center, sides)

Node = namedtuple('Node', 'score index total left path')

def make_node(index, total, left, path):
    return Node(abs(total - left), index, total, left, path)

def partial_sum(it):
    val = 0
    for i in it:
        val += i
        yield val

def pack(sides):
    half = sum(sides) / 2
    sort = list(sorted(sides, reverse=True))
    sortleft = list(reversed(list(partial_sum(reversed(sort)))))[1:] + [0]
    heap = [make_node(0, 0, half * 2, 0)]
    while heap:
        st = heapq.heappop(heap)

        if st.total == half and st.left == half:
            left = set(sort[i] for i in range(len(sides)) if (1 << i) & st.path)
            right = sides - left
            return (left, right)
        if st.index >= len(sort):
            continue

        val = sort[st.index]
        if st.total + val <= half:
            heapq.heappush(heap, make_node(
                st.index + 1,
                st.total + val,
                st.left - val,
                st.path | (1 << st.index)))
        if st.total + sortleft[st.index] >= half:
            heapq.heappush(heap, make_node(
                st.index + 1,
                st.total,
                st.left,
                st.path))

    return None

def solve(weights):
    best_qe = -1
    best_num = 0
    
    for center, sides in partitions(weights):
        packed = pack(sides)
        
        if not packed:
            continue

        num = len(center)
        qe = reduce(operator.mul, center, 1)

        if best_num == 0:
            best_num = num
            best_qe = qe

        if num > best_num:
            return best_qe
        else:
            best_qe = min(best_qe, qe)

    return best_qe

weights = set(int(l.strip()) for l in sys.stdin if l.strip())
print solve(weights)
