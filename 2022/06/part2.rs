mod common;
use std::io;
use common::find_unique;

fn main() {
    let input = io::stdin().lines()
        .flat_map(|l| l.ok())
        .map(|l| l.trim().to_string())
        .filter(|l| !l.is_empty())
        .next().expect("Expected to find a non-empty line");

    println!("{}", find_unique(input.as_bytes(), 14).expect("No start-of-packet found!"));
}
