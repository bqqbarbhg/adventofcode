import sys
from itertools import product
from cyk import CYK

rules = []

for line in sys.stdin:
     line = line.strip()
     if not line: break
     name, rest = line.split(":", 1)
     prods = rest.split("|")
     if len(prods) == 1 and prods[0].strip().startswith("\""):
         rules.append((int(name), (prods[0].strip()[1:-1],)))
     else:
         for p in prods:
             rules.append((int(name), tuple(int(v.strip()) for v in p.split())))

cyk = CYK(rules)
print(sum(1 for l in sys.stdin if cyk.match(l.strip())))
