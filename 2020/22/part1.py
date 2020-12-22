import sys

lines = [l.strip() for l in sys.stdin if l.strip()]
p1, p2 = lines.index("Player 1:"), lines.index("Player 2:")
decks = (list(map(int, lines[p1+1:p2])), list(map(int, lines[p2+1:])))

while all(decks):
    tops = list(sorted((d[0] for d in decks), reverse=True))
    decks = [d[1:] + (tops if d[0] == tops[0] else []) for d in decks]

print(sum(v*(i+1) for i,v in enumerate(reversed(sum(decks, [])))))
