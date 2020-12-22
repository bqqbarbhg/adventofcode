import sys

lines = [l.strip() for l in sys.stdin if l.strip()]
p1, p2 = lines.index("Player 1:"), lines.index("Player 2:")
decks = (tuple(map(int, lines[p1+1:p2])), tuple(map(int, lines[p2+1:])))

def play(decks):
    decks = tuple(decks)
    seen = set()
    while all(decks):
        if decks in seen: return (0, decks)
        seen.add(decks)
        tops = tuple(d[0] for d in decks)
        decks = tuple(d[1:] for d in decks)
        if all(t <= len(d) for d,t in zip(decks, tops)):
            win_i, _ = play(d[:t] for d,t in zip(decks, tops))
        else:
            win_i = tops.index(max(tops))
        tops = (tops[win_i],) + tops[:win_i] + tops[win_i+1:]
        decks = tuple(d + tops*(i==win_i) for i,d in enumerate(decks))
    return next(i for i,d in enumerate(decks) if d), decks

_, decks = play(decks)
print(sum(v*(i+1) for i,v in enumerate(reversed(sum(decks, ())))))
