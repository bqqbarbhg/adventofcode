import sys

start, periods = sys.stdin.readlines()
start = int(start)
periods = [int(b) for b in periods.split(",") if b != "x"]

def wait_time(period):
    return ((start + period - 1) // period * period) - start

best_period = min(periods, key=wait_time)
print(best_period * wait_time(best_period))
