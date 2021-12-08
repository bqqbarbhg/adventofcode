import sys

digit_leds = {
    0: "abcefg",
    1: "cf",
    2: "acdeg",
    3: "acdfg",
    4: "bcdf",
    5: "abdfg",
    6: "abdefg",
    7: "acf",
    8: "abcdefg",
    9: "abcdfg",
}

def canonical(pattern):
    return "".join(sorted(pattern))

def try_assign(patterns, digits, wires):
    if not patterns: return { }
    pattern = patterns[0]
    for digit in digits:
        leds = digit_leds[digit]
        if len(leds) != len(pattern): continue
        inc = set(leds)
        exc = set("abcdefg") - inc
        new_wires = { w: l & (inc if w in pattern else exc) for w,l in wires.items() }
        if not all(new_wires.values()): continue
        result = try_assign(patterns[1:], digits - { digit }, new_wires)
        if result is not None: return { **result, pattern: digit }
    return None

def assign(patterns):
    wires = { w: set("abcdefg") for w in "abcdefg" }
    digits = set(range(0, 9+1))
    return try_assign(patterns, digits, wires)

total = 0
for line in sys.stdin:
    patterns, digits = line.split("|")
    patterns = [canonical(pat) for pat in patterns.split()]
    digits = [canonical(dig) for dig in digits.split()]

    mapping = assign(patterns)
    value = int("".join(str(mapping[digit]) for digit in digits))
    total += value

print(total)
