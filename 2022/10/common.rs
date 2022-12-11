#[derive(Debug, Clone, Copy)]
pub enum Op {
    Noop,
    Addx(i64),
}

impl Op {
    pub fn parse(line: &str) -> Option<Op> {
        let parts: Vec<_> = line.trim().split_ascii_whitespace().collect();
        let op = match parts.as_slice() {
            &["noop"] => Op::Noop,
            &["addx", v] => Op::Addx(v.parse().ok()?),
            _ => return None,
        };
        Some(op)
    }
}

pub struct Vm {
    pub code: Vec<Op>,
    pub pc: usize,
    pub x: i64,
    pub cycle: u64,
    pub phase: u64,
}

impl Vm {
    pub fn new(code: Vec<Op>) -> Vm {
        Vm { code: code, pc: 0, x: 1, cycle: 0, phase: 0 }
    }

    fn exec(&mut self, op: Op, phase: u64) -> bool {
        match (op, phase) {
            (Op::Noop, 1) => {},
            (Op::Addx(v), 2) => { self.x += v; }
            _ => return false,
        }
        true
    }

    pub fn step(&mut self) {
        while self.exec(self.code[self.pc], self.phase) {
            self.phase = 0;
            self.pc += 1;
        }
        self.phase += 1;
        self.cycle += 1;
    }
}
