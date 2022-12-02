mod common;

use std::{io::{self, BufRead}};
use std::vec::Vec;
use common::Shape;

fn line_score(line: &str) -> Option<i32> {
    let tokens: Vec<&str> = line
        .split_ascii_whitespace()
        .take(2)
        .collect();

    let b = *tokens.get(0)?;
    let op = *tokens.get(1)?;

    let b = Shape::parse(b)?;
    let target_delta = match op {
        "X" => Some(-1),
        "Y" => Some(0),
        "Z" => Some(1),
        _ => None,
    }?;

    for a in Shape::iter() {
        let delta = (a.beats(&b) as i32) - (b.beats(a) as i32);
        if delta == target_delta {
            return Some(a.score() + 3 + delta * 3);
        }
    }

    None
}

fn main() {
    let total_score: i32 = io::stdin().lock().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| line_score(&l))
        .sum();
    println!("{}", total_score);
}
