use std::io::prelude::*;

fn exec(mem: &mut [i32]) {
    let mut ip: usize = 0;
    while ip < mem.len() {
        let op = mem[ip];
        match op {
            1 => { // Add
                let a = mem[mem[ip + 1] as usize];
                let b = mem[mem[ip + 2] as usize];
                mem[mem[ip + 3] as usize] = a + b;
                ip += 4;
            },
            2 => { // Mul
                let a = mem[mem[ip + 1] as usize];
                let b = mem[mem[ip + 2] as usize];
                mem[mem[ip + 3] as usize] = a * b;
                ip += 4;
            },
            99 => { // Halt
                return;
            },
            _ => panic!("Invalid opcode {}", op)
        }
    }
}

fn main() {
    let stdin = std::io::stdin();
    let mut mem: Vec<i32> = stdin.lock().lines()
        .flatten()
        .flat_map(|l| l.split(',').flat_map(|v| v.parse::<i32>()).collect::<Vec<i32>>())
        .collect();

    // 1202 program alarm
    mem[1] = 12;
    mem[2] = 2;

    exec(&mut mem[..]);

    println!("{}", mem[0]);
}

