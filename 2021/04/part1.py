import sys
N = 5

lines = (line.strip() for line in sys.stdin if line.strip())
calls = [int(v) for v in next(lines).split(",")]

cells = (tuple(int(v) for v in line.split()) for line in lines)
boards = list(zip(*[cells]*N)) # Silly Python golf trick for chunks of N

checks = []
checks += [[(x,y) for x in range(N)] for y in range(N)]
checks += [[(x,y) for y in range(N)] for x in range(N)]
# For diagonals: checks += [[(x,x) for x in range(N)], [(x,N-x-1) for x in range(N)]]

called = set()
for call in calls:
    called.add(call)
    for board in boards:
        for check in checks:
            if all(board[y][x] in called for x,y in check):
                numbers = [v for row in board for v in row]
                score = sum(v for v in numbers if v not in called) * call
                print(score)
                sys.exit(0)
