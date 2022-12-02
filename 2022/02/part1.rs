mod common;

use std::{io::{self, BufRead}};
use std::vec::Vec;
use common::Shape;

fn main() {
    let mut total_score = 0;

    let stdin = io::stdin();
    for line in stdin.lock().lines().flat_map(|l| l.ok()) {
        let tokens: Vec<Shape> = line
            .split_ascii_whitespace()
            .flat_map(Shape::parse)
            .take(2)
            .collect();

        if let [b, a] = tokens.as_slice() {
            let score = a.score() + 3
                + if a.beats(b) { 3 } else { 0 }
                - if b.beats(a) { 3 } else { 0 };
            total_score += score;
        }
    }

    println!("{}", total_score);
}
