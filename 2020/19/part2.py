import sys
from itertools import product
from cyk import CYK

rules = []

for line in sys.stdin:
     line = line.strip()
     if not line: break

     if line == "8: 42":
         line = "8: 42 | 42 8"
     elif line == "11: 42 31":
         line = "11: 42 31 | 42 11 31"

     name, rest = line.split(":", 1)
     prods = rest.split("|")
     if len(prods) == 1 and prods[0].strip().startswith("\""):
         rules.append((int(name), (prods[0].strip()[1:-1],)))
     else:
         for p in prods:
             rules.append((int(name), tuple(int(v.strip()) for v in p.split())))

cyk = CYK(rules)
print(sum(1 for l in sys.stdin if cyk.match(l.strip())))
