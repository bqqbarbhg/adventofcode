import re
import sys
import operator
from functools import reduce

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

own_ticket = own_tickets[0]
assert len(own_ticket) == len(fields)

any_field = reduce(operator.or_, fields.values())
field_valid = { f: set(range(len(fields))) for f in fields }
for ticket in nearby_tickets:
    if not all(v in any_field for v in ticket): continue
    for ix, val in enumerate(ticket):
        for name, vals in fields.items():
            if val in vals: continue
            field_valid[name].discard(ix)

field_index = { }
while len(field_index) < len(fields):
    num, opts, name = min((len(options), options, name) for name, options in field_valid.items())
    assert num == 1, "Might need a better algorithm.."
    del field_valid[name]
    index = next(iter(opts))
    field_index[name] = index
    for options in field_valid.values():
        options.discard(index)

print(reduce(operator.mul, (own_ticket[i] for n,i in field_index.items() if n.startswith("departure"))))
