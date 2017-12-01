#!/usr/bin/env python

from collections import namedtuple
import sys
import heapq
import copy

Effect = namedtuple('Effect', 'name duration proc')
Spell = namedtuple('Spell', 'name cost proc')

def shield_proc(state):
    state.player.frame_armor += 7

def poison_proc(state):
    state.enemy.hp -= 3

def recharge_proc(state):
    state.player.mana += 101

def magic_missile_spell(state):
    state.enemy.hp -= 4
    return True

def drain_spell(state):
    state.enemy.hp -= 2
    state.player.hp += 2
    return True

def effect_spell(effect):
    def inner_spell(state):
        if any(e.proc == effect.proc for e in state.effects):
            return False
        state.effects.append(effect)
        return True
    return inner_spell

spells = [
    Spell('Magic Missile', 53, magic_missile_spell),
    Spell('Drain', 73, drain_spell),
    Spell('Sheild', 113, effect_spell(Effect('Shield', 6, shield_proc))),
    Spell('Poison', 173, effect_spell(Effect('Poison', 6, poison_proc))),
    Spell('Recharge', 229, effect_spell(Effect('Recharge', 5, recharge_proc))),
]

class Char(object):
    def __init__(self, parent=None):
        if parent:
            self.hp = parent.hp
            self.damage = parent.damage
            self.armor = parent.armor
            self.frame_armor = parent.frame_armor
            self.mana = parent.mana
        else:
            self.hp = 0
            self.damage = 0
            self.armor = 0
            self.frame_armor = 0
            self.mana = 0

class State(object):
    def __init__(self, parent=None):
        if parent:
            self.effects = parent.effects
            self.player = Char(parent.player)
            self.enemy = Char(parent.enemy)
            self.mana_used = parent.mana_used
            self.spells_used = copy.copy(parent.spells_used)
        else:
            self.effects = []
            self.player = Char()
            self.enemy = Char()
            self.mana_used = 0
            self.spells_used = []

    def effect_tick(self):
        self.effects = [Effect(e.name, e.duration - 1, e.proc) for e in self.effects]
        self.player.frame_armor = self.player.armor
        for e in self.effects:
            e.proc(self)
        self.effects = [e for e in self.effects if e.duration > 0]

    def turn(self, spell):
        if self.player.mana < spell.cost:
            return None
        
        st = State(self)
        st.effect_tick()
        
        if not spell.proc(st):
            return None
        
        st.player.mana -= spell.cost
        st.mana_used += spell.cost
        st.spells_used.append(spell)

        st.effect_tick()

        if st.enemy.hp > 0:        
            st.player.hp -= max(1, st.enemy.damage - st.player.frame_armor)
        
        
        return st

    def __repr__(self):
        return '<State Player {}hp {}mana {}armor Enemy {}hp >'.format(self.player.hp, self.player.mana, self.player.frame_armor, self.enemy.hp)

def setup():
    args = { }
    args['Hit Points'] = 58
    args['Damage'] = 9
    for line in sys.stdin:
        if line.strip():
            k,v = (l.strip() for l in line.split(':'))
            args[k] = int(v)

    s = State()
    s.player.hp = 50
    s.player.mana = 500
    s.enemy.hp = args['Hit Points']
    s.enemy.damage = args['Damage']
    return s

Node = namedtuple('Node', 'score state')

def make_node(state):
    return Node(state.mana_used, state)

def lowest_mana(state):
    heap = [make_node(state)]
    while heap:
        st = heapq.heappop(heap)
        if st.state.enemy.hp <= 0:
            return st.state

        for s in spells:
            nx = st.state.turn(s)
            if nx and nx.player.hp > 0:
                heapq.heappush(heap, make_node(nx))

st = lowest_mana(setup())
print st.mana_used

