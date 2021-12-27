from dataclasses import dataclass, replace, field
from typing import Set, Optional

REGRM = "REGRM"
REXW = "REXW"
IMM8 = "IMM8"
IMM16 = "IMM16"
IMM32 = "IMM32"
IMM64 = "IMM64"
IMMSZ = "IMMSZ"
IMM8OP2 = "IMM8OP2"
IMM32JCC = "IMM32JCC"
IMM1OP10 = "IMM1OP10"

imm_flags = {
    1: IMM8,
    2: IMM16,
    4: IMM32,
    8: IMM64,
}

@dataclass
class Inst:
    name: str    # Mnemonic of the instruction
    args: str    # Arguments eg. mr
    opts: str    # Flags eg. sdwq
    op: int      # Instruction with prefixes, eg. adds 0xf3_0f_58

    reg: Optional[int] = None # REG field specifies operation
    flags: Set[str] = field(default_factory=set)
    infix: str = "" # Instruction infix
    suffix: str = "" # Instruction suffix
    regsz: int = 4 # Operand size if not provided
    immsz: int = 0 # Immediate argument size
    rel: bool = False # Immediate is relative
    unsized: bool = False # No size suffix
    imm64: bool = False # Allow 64-bit immediate

instructions = [
    Inst("add", "mr", "sdwq", 0x00),
    Inst("or",  "mr", "sdwq", 0x08),
    Inst("adc", "mr", "sdwq", 0x10),
    Inst("sbb", "mr", "sdwq", 0x18),
    Inst("and", "mr", "sdwq", 0x20),
    Inst("sub", "mr", "sdwq", 0x28),
    Inst("xor", "mr", "sdwq", 0x30),
    Inst("cmp", "mr", "sdwq", 0x38),

    Inst("add", "mi", "swqi", 0x80, reg=0, flags={IMM8OP2}),
    Inst("or",  "mi", "swqi", 0x80, reg=1, flags={IMM8OP2}),
    Inst("adc", "mi", "swqi", 0x80, reg=2, flags={IMM8OP2}),
    Inst("sbb", "mi", "swqi", 0x80, reg=3, flags={IMM8OP2}),
    Inst("and", "mi", "swqi", 0x80, reg=4, flags={IMM8OP2}),
    Inst("sub", "mi", "swqi", 0x80, reg=5, flags={IMM8OP2}),
    Inst("xor", "mi", "swqi", 0x80, reg=6, flags={IMM8OP2}),
    Inst("cmp", "mi", "swqi", 0x80, reg=7, flags={IMM8OP2}),

    Inst("test", "rm", "swq", 0x84),
    Inst("xchg", "rm", "swq", 0x86),
    Inst("lea",  "rm", "wq", 0x8d),

    Inst("imul", "rm", "wq", 0x0f_af),
    Inst("imul", "rmi", "wqi", 0x69),

    Inst("jmp", "i", "i", 0xe9, flags={IMM8OP2}, rel=True, unsized=True),
    Inst("jcc", "ci", "i", 0x70, flags={IMM32JCC}, rel=True, unsized=True),
    Inst("setcc", "cm", "", 0x0f_90, regsz=1),
    Inst("cmovcc", "crm", "wq", 0x0f_40),

    Inst("mov", "rm", "sdwq", 0x88),
    Inst("mov", "li", "i", 0xb0, regsz=1),
    Inst("mov", "li", "wqi", 0xb8, imm64=True),
    Inst("mov", "mi", "swqi", 0xc6),

    Inst("rol", "mi", "swqi", 0xc0, reg=0, immsz=1, flags={IMM1OP10}),
    Inst("ror", "mi", "swqi", 0xc0, reg=1, immsz=1, flags={IMM1OP10}),
    Inst("rcl", "mi", "swqi", 0xc0, reg=2, immsz=1, flags={IMM1OP10}),
    Inst("rcr", "mi", "swqi", 0xc0, reg=3, immsz=1, flags={IMM1OP10}),
    Inst("shl", "mi", "swqi", 0xc0, reg=4, immsz=1, flags={IMM1OP10}),
    Inst("shr", "mi", "swqi", 0xc0, reg=5, immsz=1, flags={IMM1OP10}),
    Inst("sal", "mi", "swqi", 0xc0, reg=6, immsz=1, flags={IMM1OP10}),
    Inst("sar", "mi", "swqi", 0xc0, reg=7, immsz=1, flags={IMM1OP10}),

    Inst("rol", "m", "swq", 0xd2, reg=0, suffix="_cl"),
    Inst("ror", "m", "swq", 0xd2, reg=1, suffix="_cl"),
    Inst("rcl", "m", "swq", 0xd2, reg=2, suffix="_cl"),
    Inst("rcr", "m", "swq", 0xd2, reg=3, suffix="_cl"),
    Inst("shl", "m", "swq", 0xd2, reg=4, suffix="_cl"),
    Inst("shr", "m", "swq", 0xd2, reg=5, suffix="_cl"),
    Inst("sal", "m", "swq", 0xd2, reg=6, suffix="_cl"),
    Inst("sar", "m", "swq", 0xd2, reg=7, suffix="_cl"),

    Inst("test", "mi", "swqi", 0xf6, reg=0),
    Inst("not",  "m",  "swq", 0xf6, reg=2),
    Inst("neg",  "m",  "swq", 0xf6, reg=3),
    Inst("mul",  "m",  "swq", 0xf6, reg=4, infix="_ax"),
    Inst("imul", "m",  "swq", 0xf6, reg=5, infix="_ax"),
    Inst("div",  "m",  "swq", 0xf6, reg=6, infix="_ax"),
    Inst("idiv", "m",  "swq", 0xf6, reg=7, infix="_ax"),

    Inst("call", "i", "i", 0xe8, immsz=4, rel=True, unsized=True),
    Inst("ret", "", "", 0xc3, unsized=True),

    Inst("inc", "m", "swq", 0xfe, reg=0),
    Inst("dec", "m", "swq", 0xfe, reg=1),
    Inst("call", "m", "", 0xff, reg=2, regsz=8, unsized=True),
    Inst("jmp", "m", "", 0xff, reg=4, regsz=8, unsized=True),
]

def num_bytes(word):
    n = 1
    word >>= 8
    while word:
        n += 1
        word >>= 8
    return n

def add_prefix(word, prefix):
    for n in range(3, 8):
        if word & (0xff << (n*8)) == 0:
            return word | (prefix << (n*8))
    raise ValueError("Too mankby prefixes")

def expand_s(instructions):
    for inst in instructions:
        if "s" in inst.opts:
            yield replace(inst, op=inst.op, regsz=1)
            yield replace(inst, op=inst.op+1, regsz=4)
        else:
            yield inst

def expand_d(instructions):
    for inst in instructions:
        if "d" in inst.opts:
            yield replace(inst, op=inst.op, args="mr")
            yield replace(inst, op=inst.op+2, args="rm")
        else:
            yield inst

def expand_w(instructions):
    for inst in instructions:
        if "w" in inst.opts and inst.regsz == 4:
            op = add_prefix(inst.op, 0x66)
            yield replace(inst, op=op, regsz=2)
        yield inst

def expand_q(instructions):
    for inst in instructions:
        yield inst
        if "q" in inst.opts and inst.regsz == 4:
            yield replace(inst, regsz=8, flags=inst.flags | { REXW })

def expand_imm(instructions):
    for inst in instructions:
        if "i" in inst.opts:
            immsz = inst.immsz
            if immsz == 0:
                immsz = inst.regsz
                if not inst.imm64:
                    immsz = min(immsz, 4)
            iflags = set(inst.flags)
            flags = set(inst.flags) | { IMMSZ }
            if immsz == 1 and IMM8OP2 in flags:
                iflags.remove(IMM8OP2)
                flags.remove(IMM8OP2)
            if IMM8OP2 in flags:
                flags.remove(IMM8OP2)
                yield replace(inst, flags=iflags, immsz=immsz)
                yield replace(inst, flags=flags, immsz=1, op=inst.op+2)
                yield replace(inst, flags=flags, immsz=immsz)
            elif IMM32JCC in flags:
                flags.remove(IMM32JCC)
                yield replace(inst, flags=iflags, immsz=1)
                yield replace(inst, flags=flags, immsz=1)
                yield replace(inst, flags=flags, immsz=immsz, op=0x0f_00|(inst.op+0x10))
            elif IMM1OP10 in flags:
                flags.remove(IMM1OP10)
                yield replace(inst, flags=iflags, immsz=immsz)
                yield replace(inst, flags=flags, immsz=immsz)
            else:
                yield replace(inst, flags=iflags, immsz=immsz)
                yield replace(inst, flags=flags, immsz=immsz)
        else:
            yield inst

instructions = expand_s(instructions)
instructions = expand_w(instructions)
instructions = expand_q(instructions)
instructions = expand_d(instructions)
instructions = expand_imm(instructions)
instructions = list(instructions)

sz_suffix = { 1: "8", 2: "16", 4: "32", 8: "64" }
for inst in instructions:
    for name in inst.name.split():
        if inst.regsz and not inst.unsized:
            name += sz_suffix[inst.regsz]
        if inst.infix:
            name += inst.infix
        suffixes = ""
        immsz = inst.immsz
        params = ["a"]
        spec = frozenset(inst.args)
        for arg in inst.args:
            if arg == "r":
                suffixes += "_reg"
                params.append("reg")
            elif arg == "l":
                suffixes += "_lowreg"
                params.append("lowreg")
            elif arg == "m":
                suffixes += "_rm"
                params.append("rm")
            elif arg == "c":
                params.append("cc")
            elif arg == "i":
                params.append("imm")
                if IMMSZ in inst.flags:
                    if inst.rel:
                        suffixes += f"_rel{immsz * 8}"
                    else:
                        suffixes += f"_imm{immsz * 8}"
                else:
                    if inst.rel:
                        suffixes += "_rel"
                    else:
                        suffixes += "_imm"
        if inst.suffix:
            suffixes += inst.suffix
        params_str = ", ".join(params)

        flags = set(inst.flags)
        if immsz:
            flags |= { imm_flags[immsz] }
        if "m" in inst.args:
            flags |= { REGRM }
        flags = "|".join(f"X64_F_{f}" for f in flags)
        if not flags: flags = "0"

        reg = inst.reg or 0

        nb = num_bytes(inst.op)
        op = f"0x{inst.op:0{nb*2}x}"
        if spec == frozenset(""):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, 0, 0, 0)")
        elif spec == frozenset("i"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, 0, 0, (imm))")
        elif spec == frozenset("m"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, {reg}, (rm), 0)")
        elif spec == frozenset("rm"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, (reg), (rm), 0)")
        elif spec == frozenset("mi"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, {reg}, (rm), (imm))")
        elif spec == frozenset("ri"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, (reg), 0, (imm))")
        elif spec == frozenset("li"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), x64_op_lowreg({op}, (lowreg)), {flags}, 0, 0, (imm))")
        elif spec == frozenset("rmi"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), {op}, {flags}, (reg), (rm), (imm))")
        elif spec == frozenset("ci"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), x64_op_cc({op}, (cc)), {flags}, 0, 0, (imm))")
        elif spec == frozenset("cm"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), x64_op_cc({op}, (cc)), {flags}, {reg}, (rm), 0)")
        elif spec == frozenset("crm"):
            print(f"#define x64_{name}{suffixes}({params_str}) x64_emit((a), x64_op_cc({op}, (cc)), {flags}, (reg), (rm), 0)")
        else:
            raise ValueError("Unknown spec: " + str(spec))
