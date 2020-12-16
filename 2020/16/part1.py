import re
import sys
import operator
from functools import reduce
from itertools import chain

fields = { }
own_tickets = []
nearby_tickets = []

for line in sys.stdin:
    line = line.strip()
    m = re.match(r"\s*(.*):\s*(\d+)-(\d+)\s*or\s*(\d+)-(\d+)", line)
    if m:
        name, lo0, hi0, lo1, hi1 = m.groups()
        fields[name] = set(range(int(lo0), int(hi0)+1)) | set(range(int(lo1), int(hi1)+1))
    elif line == "your ticket:":
        tickets = own_tickets
    elif line == "nearby tickets:":
        tickets = nearby_tickets
    elif line == "":
        continue
    else:
        tickets.append(tuple(int(v) for v in line.split(",")))

any_field = reduce(operator.or_, fields.values())
print(sum(v for v in chain(*nearby_tickets) if v not in any_field))
