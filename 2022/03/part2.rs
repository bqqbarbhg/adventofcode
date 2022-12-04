use std::{io::{self, BufRead}};
use std::vec::Vec;

fn char_prio(ch: char) -> u32 {
    match ch {
        'a'..='z' => 1  + (ch as u32) - ('a' as u32),
        'A'..='Z' => 27 + (ch as u32) - ('A' as u32),
        _ => panic!("Unexpected character {}", ch),
    }
}

fn main() {
    let mut total_prio = 0;

    let input: Vec<String> = io::stdin().lock().lines()
        .flat_map(|l| l.ok())
        .map(|l| String::from(l.trim()))
        .filter(|l| !l.is_empty())
        .collect();

    for chunk in input.chunks(3) {
        let common_bits = chunk.iter()
            .map(|l| l.chars().map(char_prio).fold(0u64, |a, b| a | 1u64 << b))
            .fold(!0u64, |a, b| a & b);
        let prio = common_bits.trailing_zeros();
        total_prio += prio;
    }

    println!("{}", total_prio);
}

