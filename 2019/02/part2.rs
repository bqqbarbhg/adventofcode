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
    let target = std::env::args().nth(1).unwrap().parse::<i32>().unwrap();

    let stdin = std::io::stdin();
    let mem_original: Vec<i32> = stdin.lock().lines()
        .flatten()
        .flat_map(|l| l.split(',').flat_map(|v| v.parse::<i32>()).collect::<Vec<i32>>())
        .collect();

    for noun in 0..=99 {
        for verb in 0..=99 {
            let mut mem = mem_original.clone();
            mem[1] = noun;
            mem[2] = verb;

            exec(&mut mem[..]);

            if mem[0] == target {
                println!("{}", 100 * noun + verb);
                return;
            }
        }
    }
}


