#!/usr/bin/env python
import sys
import itertools
import operator
import heapq
import copy
from collections import namedtuple

def partitions(weights, ngrp, num):
    onepart = sum(weights) / ngrp
    for c in itertools.combinations(weights, num):
        if sum(c) == onepart:
            center = set(c)
            sides = weights - center
            yield (center, sides)

Node = namedtuple('Node', 'score index total left path')

def make_node(index, total, left, path):
    return Node(abs(left), index, total, left, path)

def partial_sum(it):
    val = 0
    for i in it:
        val += i
        yield val

def pack(sides, ngrp):
    onepart = sum(sides) / ngrp
    sort = list(sorted(sides, reverse=True))
    sortleft = list(reversed(list(partial_sum(reversed(sort)))))
    heap = [make_node(0, [0]*ngrp, sum(sides), [])]
    while heap:
        st = heapq.heappop(heap)

        if all(v == onepart for v in st.total):
            lists = tuple([] for _ in range(ngrp))
            for i,s in zip(st.path, sort):
                lists[i].append(s)
            return tuple(lists)
        
        if st.index >= len(sort):
            continue

        val, left = sort[st.index], sortleft[st.index]
        if any(v + left < onepart for v in st.total):
            continue

        for i,v in enumerate(st.total):
            if v + val <= onepart:
                total = copy.copy(st.total)
                total[i] += val

                heapq.heappush(heap, make_node(
                    st.index + 1,
                    total,
                    st.left - val,
                    st.path + [i]))

    return None

def solve(weights, ngrp):
    for num in range(1, len(weights)):
        parts = list(partitions(weights, ngrp, num))
        partqe = sorted(zip((reduce(operator.mul, c[0], 1) for c in parts), parts))
        for qe,p in partqe:
            pk = pack(p[1], ngrp - 1)
            if pk:
                return (qe,(p[0],) + pk)

    return None

weights = set(int(l.strip()) for l in sys.stdin if l.strip())
print solve(weights,4)[0]
