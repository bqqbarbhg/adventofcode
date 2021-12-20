import sys
from collections import defaultdict

def add(a, b):
    return (a[0]+b[0], a[1]+b[1])

neighbors = [
    (-1,-1), ( 0,-1), (+1,-1),
    (-1, 0), ( 0, 0), (+1, 0),
    (-1,+1), ( 0,+1), (+1,+1),
]

def step(state, rule):
    masks = defaultdict(int)
    for pos in state:
        for ix, delta in enumerate(neighbors):
            masks[add(pos, delta)] |= 1 << ix
    return [k for k,v in masks.items() if rule[v]]

rule, _, *map = (s.strip() for s in sys.stdin)
rule = [c == "#" for c in rule]
state = [(x,y) for y,line in enumerate(map) for x,c in enumerate(line) if c == "#"]

rule_a = [not b for b in rule]
rule_b = list(reversed(rule))

state = step(state, rule_a)
state = step(state, rule_b)

print(len(state))