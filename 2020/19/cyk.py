from collections import defaultdict
from itertools import count, groupby

def make_rule_gen(rules):
    return count(max(r for r,_ in rules) + 1)

def normalize_term(rules, rule_gen):
    term_to_rule = { }

    def replace_terminal(p):
        if not isinstance(p, str): return p
        if p in term_to_rule:
            return term_to_rule[p]
        else:
            r = next(rule_gen)
            term_to_rule[p] = r
            return r

    for r, ps in rules:
        yield (r, tuple(replace_terminal(p) for p in ps))
    for t, r in term_to_rule.items():
        yield (r, t)

def normalize_bin(rules, rule_gen):
    for r, ps in rules:
        while len(ps) > 2:
            s = next(rule_gen)
            yield (r, (ps[0], s))
            r, ps = s, ps[1:]
        yield (r, ps)

def normalize_unit(rules):
    rule_map = defaultdict(set)
    for r, ps in rules:
        rule_map[r].add(ps)

    def non_unit_rules(r):
        for ps in rule_map[r]:
            if len(ps) == 1 and isinstance(ps[0], int):
                yield from non_unit_rules(ps[0])
            else:
                yield ps

    for r in rule_map:
        for ps in non_unit_rules(r):
            yield (r, ps)

def normalize_clean(rules):
    rule_to_prod = defaultdict(set)
    for r, ps in rules:
        rule_to_prod[r].add(ps)
    rule_to_prod = { r: frozenset(pss) for r,pss in rule_to_prod.items() }
    prod_to_rule = { pss: r for r,pss in rule_to_prod.items() }
    prod_to_rule[rule_to_prod[0]] = 0

    name_map = { }
    clean_rules = []
    def walk_rename(p):
        if isinstance(p, str): return p
        if p in name_map: return name_map[p]
        r = prod_to_rule[rule_to_prod[p]]
        processed = r in name_map
        name_map[p] = r
        if processed: return r
        for ps in rule_to_prod[r]:
            clean_rules.append((r, tuple(walk_rename(p) for p in ps)))
        return r
    walk_rename(0)
    return clean_rules

def normalize(rules):
    rule_gen = make_rule_gen(rules)
    rules = normalize_term(rules, rule_gen)
    rules = normalize_bin(rules, rule_gen)
    rules = normalize_unit(rules)
    rules = normalize_clean(rules)
    return list(rules)

class CYK:
    def __init__(self, rules):
        rules = normalize(rules)
        self.ts = {}
        self.ps = []
        for r, ps in rules:
            if len(ps) == 1 and isinstance(ps[0], str):
                self.ts.setdefault(ps[0], []).append(r)
            else:
                self.ps.append((r, ps[0], ps[1]))
            
    def match(self, src):
        pset = set()
        sset = set()
        n = len(src)

        # Initialize terminal rules
        # a(st[s]) <- "t"
        for s,t in enumerate(src):
            for r in self.ts[t]:
                pset.add((s, s+1, r))
                sset.add(r)

        # Iteratively concatenate larger rules
        # a(st[s:s+l]) <- b(st[s:s+p]) c(st[s+p:s+l])
        for l in range(2, n + 1):
            for a,b,c in self.ps:
                if b not in sset or c not in sset: continue
                for s in range(0, n - l + 1):
                    for p in range(0, l):
                        if (s, s+p, b) in pset and (s+p, s+l, c) in pset:
                            pset.add((s, s+l, a))
                            sset.add(a)

        return (0, n, 0) in pset
