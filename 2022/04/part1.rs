mod common;
use std::{io::{self, BufRead}};
use common::parse_line;

fn main() {
    let input = io::stdin().lock().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| parse_line(&l));

    let mut total_overlaps = 0;
    for (a, b) in input {
        if a.contains(&b) || b.contains(&a) {
            total_overlaps += 1;
        }
    }
    println!("{}", total_overlaps);
}
