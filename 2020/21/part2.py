import sys

def strip(strs):
    return (s.strip() for s in strs if s.strip())

def parse_foods(lines):
    for line in lines:
        igs, ags = line[:-1].split(" (contains ")
        yield set(strip(igs.split())), set(strip(ags.split(",")))

def assign(opts, assignments={}):
    if not opts: return assignments
    _, name, values = min((len(vs), n, vs) for n,vs in opts.items())
    for v in values:
        opts_left = dict((n, vs - { v }) for n,vs in opts.items() if n != name)
        if not all(opts_left.values()): return
        res = assign(opts_left, assignments | { name: v }) 
        if res: return res

ag_ig_opts = { }
for igs, ags in parse_foods(strip(sys.stdin)):
    for ag in ags:
        ag_ig_opts[ag] = ag_ig_opts.get(ag, igs) & igs

ag_igs = assign(ag_ig_opts)
print(",".join(v for _,v in sorted(ag_igs.items())))
