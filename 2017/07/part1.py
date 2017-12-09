#!/usr/bin/env python3
import sys
import shared

nodes = shared.parse_nodes(sys.stdin)

for name, node in nodes.items():
    if not node.parent:
        print(name)

