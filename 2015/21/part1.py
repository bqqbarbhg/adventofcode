#!/usr/bin/env python
from collections import namedtuple
import itertools
import operator
import sys

Char = namedtuple('Char', 'hp damage armor')
Item = namedtuple('Item', 'cost damage armor')

null = Item(0, 0, 0)

weapons = [
    Item(8,  4, 0),
    Item(10, 5, 0),
    Item(25, 6, 0),
    Item(40, 7, 0),
    Item(74, 8, 0),
]

armor = [
    Item(13,  0, 1),
    Item(31,  0, 2),
    Item(53,  0, 3),
    Item(75,  0, 4),
    Item(102, 0, 5),
]

rings = [
    Item(25,  1, 0),
    Item(50,  2, 0),
    Item(100, 3, 0),
    Item(20,  0, 1),
    Item(40,  0, 2),
    Item(80,  0, 3),
]

def additem(char, item):
    return Char(char.hp, char.damage + item.damage, char.armor + item.armor)

def attack(target, attacker):
    damage = max(1, attacker.damage - target.armor)
    return Char(target.hp - damage, target.damage, target.armor)

def sim(items, boss):
    player = reduce(additem, items, Char(100, 0, 0))

    while True:
        boss = attack(boss, player)
        if boss.hp <= 0: return True
        player = attack(player, boss)
        if player.hp <= 0: return False

def buildcosts(builds, boss):
    for b in builds:
        items = [b[0], b[1]] + list(b[2])
        if sim(items, boss):
            yield sum(i.cost for i in items)

builds = itertools.product(
    weapons,
    armor + [null],
    itertools.combinations(rings + [null] * 2, 2))

stats = { }
for line in sys.stdin:
    if not line.strip():
        continue

    stat, val = (l.strip() for l in line.split(':'))
    stats[stat] = int(val)

boss = Char(stats['Hit Points'], stats['Damage'], stats['Armor'])
print min(buildcosts(builds, boss))


