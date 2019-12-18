use std::io::prelude::*;

fn main() {
    let stdin = std::io::stdin();
    let sum: i32 = stdin.lock().lines()
        .flatten()
        .flat_map(|l| l.parse::<i32>())
        .map(|n| n / 3 - 2)
        .sum();
    println!("{}\n", sum);
}

