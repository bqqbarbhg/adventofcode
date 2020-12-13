import sys
from functools import reduce

_, periods = sys.stdin.readlines()
periods = [(int(b) if b != "x" else 1) for b in periods.split(",")]

def gcd_ext(a, b):
    if b == 0:
        return a, 1, 0
    g, x, y, = gcd_ext(b, a % b)
    return g, y, x - y * (a // b)

def common_period(a_pair, pb):
    oa, pa = a_pair
    # Diophantine equation:
    #   oa + x*pa + 1 = y*pb
    #   x*pa - y*pb = oa + 1
    c = oa + 1
    g, xg, yg = gcd_ext(pa, pb)
    assert c % g == 0
    x0, y0 = xg * c // g, -yg * c // g
    xp, yp = pb // g, pa // g
    xk = (oa - x0*pa + (pa*xp - 1)) // (pa*xp)
    yk = (y0 + yp - 1) // yp
    k = max(xk, yk)
    oc = (y0 - k*yp) * -pb
    pc = yp * pb
    return oc % pc, pc

solution = reduce(common_period, periods, (0, 1))
print(solution[0] - (len(periods) - 1))
