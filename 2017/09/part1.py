#!/usr/bin/env python3
import sys

garbage = { }
escape = { }
group = { }

group_depth = 0
total_score = 0
def push_group():
    global state, group_depth, total_score
    group_depth += 1
    total_score += group_depth
    state = group

def pop_group():
    global state, group_depth
    group_depth -= 1
    state = group

garbage['>'] = group
garbage['!'] = escape
garbage[''] = garbage

escape[''] = garbage

group['<'] = garbage
group['{'] = push_group
group['}'] = pop_group
group[''] = group

state = group

for c in sys.stdin.read():
    move = state.get(c) or state.get('')
    if isinstance(move, dict):
        state = move
    else:
        move()

print(total_score)
