#!/usr/bin/env python3
import sys

garbage = { }
escape = { }
group = { }

total_score = 0

def count_garbage():
    global total_score
    total_score += 1
    state = garbage

garbage['>'] = group
garbage['!'] = escape
garbage[''] = count_garbage

escape[''] = garbage

group['<'] = garbage
group[''] = group

state = group

for c in sys.stdin.read():
    move = state.get(c) or state.get('')
    if isinstance(move, dict):
        state = move
    else:
        move()

print(total_score)
