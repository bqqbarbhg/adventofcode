import re

re_token = re.compile(r"\s*([a-z]+|[0-9]+|[,.])")

class Scanner:
    def __init__(self, source):
        self.source = source
        self.prev = ""
        self.token = ""
        self.pos = 0
        self.scan()

    def scan(self):
        match = re_token.match(self.source, self.pos)
        self.prev = self.token
        if match:
            self.token = match.group(1)
            self.pos = match.end()
        else:
            self.token = ""
        return self.prev

def parse_bag(s):
    colors = []
    while s.scan() not in ("bag", "bags"):
        colors.append(s.prev)
    return tuple(colors)

def parse_bag_count(s):
    count = int(s.scan())
    return (parse_bag(s), count)

def parse_count_list(s):
    if s.token == "no":
        for word in ("no", "other", "bags", "."):
            assert s.scan() == word
        return {}
    counts = [parse_bag_count(s)]
    while s.token == ",":
        s.scan()
        counts.append(parse_bag_count(s))
    assert s.scan() == "."
    return dict(counts)

def parse_spec(s):
    bag = parse_bag(s)
    assert s.scan() == "contain"
    counts = parse_count_list(s)
    return (bag, counts)

def parse_top(s):
    specs = []
    while s.token:
        specs.append(parse_spec(s))
    return dict(specs)

def parse(source):
    return parse_top(Scanner(source))
