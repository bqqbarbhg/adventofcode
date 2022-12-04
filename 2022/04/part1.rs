mod common;
use std::{io::{self, BufRead}};
use common::parse_line;

fn main() {
    let input = io::stdin().lock().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| parse_line(&l));

    let result = input
        .filter(|(a, b)| a.contains(&b) || b.contains(&a))
        .count();
    println!("{}", result);
}
