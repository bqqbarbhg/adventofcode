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

def is_winning(called, board):
    for check in checks:
        if all(board[y][x] in called for x,y in check):
            return True
    return False

def get_score(call, called, board):
    numbers = [v for row in board for v in row]
    return sum(v for v in numbers if v not in called) * call

called = set()
for call in calls:
    called.add(call)
    if len(boards) == 1 and is_winning(called, boards[0]):
        print(get_score(call, called, boards[0]))
        break
    boards = [b for b in boards if not is_winning(called, b)]
