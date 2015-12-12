#!/usr/bin/env python
import sys
import re
from collections import defaultdict, namedtuple
import heapq

class Node(object):
	def __init__(self):
		self.edges = []
		self.min_edge = 0.0
		self.mask = 0

graph = defaultdict(Node)

RE_LINE = re.compile(r'(\w+)\s+to\s+(\w+)\s*=\s*(\d+)')
for line in sys.stdin:
	parts = RE_LINE.match(line)
	if not parts: continue

	distance = int(parts.group(3))
	a = graph[parts.group(1)]
	b = graph[parts.group(2)]
	a.edges.append((b, distance))
	b.edges.append((a, distance))

for index, node in enumerate(graph.values()):
	node.min_edge = min(edge[1] for edge in node.edges)
	node.mask = 1 << index


State = namedtuple("State", "node mask distance estimate path")

def astar(graph):
	full_mask = (1 << len(graph)) - 1
	graph_min = sum(node.min_edge for node in graph.values())

	work = [(graph_min, State(node, node.mask, 0, graph_min - node.min_edge, [node])) for node in graph.values()]
	heapq.heapify(work)

	while work:
		state = heapq.heappop(work)[1]
		if state.mask == full_mask:
			return (state.path, state.distance)

		for edge in state.node.edges:
			node = edge[0]
			if node.mask & state.mask:
				continue
			mask = state.mask | node.mask
			distance = state.distance + edge[1]
			estimate = state.estimate - node.min_edge
			path = state.path + [node]
			new_state = State(node, mask, distance, estimate, path)
			heuristic = distance + estimate
			heapq.heappush(work, (heuristic, new_state))

	return None

path = astar(graph)
print path[1]

