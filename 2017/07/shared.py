import re
from collections import defaultdict

class Node(object):
    def __init__(self):
        self.children = []
        self.parent = None
        self.weight = -1
        self.name = ''
    def set_parent(self, parent):
        self.parent = parent
        parent.children.append(self)
    def set_weight(self, weight):
        self.weight = weight

def parse_nodes(lines):
    nodes = defaultdict(Node)
    LINE = re.compile(r'^\s*(\w+)\s*\((\d+)\)\s*(->\s*(.*))?$')
    for line in lines:
        m = LINE.match(line)
        if not m: continue
        name, weight, children = m.group(1), int(m.group(2)), m.group(4)
        parent = nodes[name]
        parent.name = name
        parent.set_weight(weight)
        if children:
            children = (c.strip() for c in children.split(','))
            for cname in children:
                child = nodes[cname]
                child.name = cname
                child.set_parent(parent)
    return nodes

