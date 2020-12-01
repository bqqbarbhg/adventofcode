use std::io::{self, BufRead};

fn main() {
    let stdin = io::stdin();
    let goal = 2020;

    let input = stdin.lock().lines()
        .flat_map(|line| line.ok())
        .flat_map(|line| line.parse::<u32>())
        .filter(|num| num <= &goal)
        .collect::<Vec<u32>>();

    for (ai, av) in input.iter().enumerate() {
        for (bi, bv) in input.iter().skip(ai).enumerate() {
            for cv in input.iter().skip(ai + bi) {
                if av + bv + cv == goal {
                    println!("{}", av * bv * cv);
                    return
                }
            }
        }
    }
}

