#!/usr/bin/env python

import sys
from collections import defaultdict, namedtuple
import re
import heapq

def replace(molecule, replacements):
    length = len(molecule)
    for start in range(length):
        for src,dst in replacements.items():
            if start + len(src) > length:
                continue
            if molecule[start:start+len(src)] == src:
                for d in dst:
                    yield molecule[:start] + d + molecule[start+len(src):]

def reverse_repl(replacements):
    repl = defaultdict(list)
    for src, dst in replacements.items():
        for d in dst:
            repl[d].append(src)
    return repl

replacements = defaultdict(list)

for line in (l.strip() for l in sys.stdin):
    if not line: break
    src, dst = tuple(line.split(' => '))
    replacements[src].append(dst)

target = next(sys.stdin).strip()

rev = reverse_repl(replacements)

tokens = re.sub(r'([A-Z])', ' \\1', target).split()

nodes = list()
stack = [[]]
literal = ''

for t in tokens:
    if t == 'Rn':
        if literal:
            stack[-1].append(literal)
        literal = ''
        stack.append([])
    elif t == 'Ar':
        if literal:
            stack[-1].append(literal)
        literal = ''
        n = stack.pop()
        stack[-1].append(n)
    else:
        literal += t
if literal:
    stack[-1].append(literal)

nodes = stack[0]

def print_nodes(nodes):
    total = ''
    for n in nodes:
        if type(n) == list:
            total += 'Rn' + print_nodes(n) + 'Ar'
        else:
            total += n
    return total

def solve(mol):
    steps = { }
    prev = set([mol])
    for step in range(1000):
        cur = set()
        for p in prev:
            if p not in steps:
                steps[p] = step
            cur.update(replace(p, rev))
        prev = cur
    return steps

def to_perms(nodes):
    for n in nodes:
        if type(n) == list:
            solved = solve(print_nodes(n))
            filtered = { k: v for k,v in solved.items() if 'Rn' not in k }
            yield filtered
        else:
            yield n

def print_perms(perms):
    if len(perms) == 0:
        yield ''

    for p in print_perms(perms[1:]):
        if type(perms[0]) == dict:
            for k,v in perms[0].items():
                yield 'Rn' + k + 'Ar' + p
        else:
            yield perms[0] + p

def make_sub_perms(perms):
    sub_perms = []
    for p in perms:
        sub_perms.append(p)
        if type(p) == dict:
            yield sub_perms
            sub_perms = []
    if sub_perms:
        yield sub_perms

def list_sub_perms(perms):
    start = ''.join(perms[:-1])
    for p in perms[-1]:
        yield start + 'Rn' + p + 'Ar'


def count_perms(perms):
    num = 1
    for i,p in enumerate(perms):
        if type(p) == dict:
            num *= len(p)
    return num

State = namedtuple('State', 'score prefix pos steps')

def make_state(prefix, pos, steps):
    score = len(prefix) + steps - pos * 16
    return State(score, prefix, pos, steps)

def astar_perms(perms):
    heap = [make_state([], 0, 0)]
    while heap:
        state = heapq.heappop(heap)

        if state.pos == len(perms):
            yield (state.prefix, state.steps)
            continue

        perm = perms[state.pos]
        if type(perm) == dict:
            for pre, step in perm.items():
                heapq.heappush(heap, make_state(
                    state.prefix + ['Rn' + pre + 'Ar'],
                    state.pos + 1,
                    state.steps + step))
        else:
            heapq.heappush(heap, make_state(
                state.prefix + [perm],
                state.pos + 1,
                state.steps))

def make_state_rec(prefix, pos, steps):
    score = len(prefix) + steps - pos * 16
    return State(score, prefix, pos, steps)

def astar_rec(chunks, basestep):
    heap = [make_state('', 0, basestep)]
    while heap:
        state = heapq.heappop(heap)

        if state.pos == len(chunks):
            yield (state.prefix, state.steps)
            continue

        slv = solve(state.prefix + chunks[state.pos])
        for k,v in slv.items():
            heapq.heappush(heap, make_state(k, state.pos + 1, state.steps + v))
        

def double_astar(perms):
    for p in astar_perms(perms):
        for pp in astar_rec(p[0], p[1]):
            yield pp

def greedy(nodes):
    for node in nodes:
        perms = list(to_perms(node))
        pre = next(double_astar(perms))
        if type(node) == list:
            pre = ('Rn' + pre[0] + 'Ar', pre[1])
        yield pre

solution = list(greedy(nodes))
molecule = ''.join(s[0] for s in solution)
base_steps = sum(s[1] for s in solution)

final = solve(molecule)

print final['e'] + base_steps
