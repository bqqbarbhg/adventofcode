use std::{io::{self, BufRead}};

fn char_prio(ch: char) -> u32 {
    match ch {
        'a'..='z' => 1  + (ch as u32) - ('a' as u32),
        'A'..='Z' => 27 + (ch as u32) - ('A' as u32),
        _ => panic!("Unexpected character {}", ch),
    }
}

fn main() {
    let mut total_prio = 0;

    let stdin = io::stdin();
    for line in stdin.lock().lines().flat_map(|l| l.ok()) {
        let line = line.trim();
        if line.is_empty() { continue }

        let (a, b) = line.split_at(line.len() / 2);
        let a_bits = a.chars().map(char_prio).fold(0u64, |a, b| a | 1u64 << b);
        let b_bits = b.chars().map(char_prio).fold(0u64, |a, b| a | 1u64 << b);
        let common_bits = a_bits & b_bits;
        let prio = common_bits.trailing_zeros();
        total_prio += prio;
    }

    println!("{}", total_prio);
}
