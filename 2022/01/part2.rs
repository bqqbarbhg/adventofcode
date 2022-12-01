use std::{io::{self, BufRead}, iter};
use std::vec::Vec;

fn main() {
    let mut top: Vec<i32> = Vec::new();
    let mut total: i32 = 0;
    let stdin = io::stdin();
    let line_iter = stdin.lock().lines().map(|l| l.unwrap());
    let iter = line_iter.chain(iter::once(String::new()));
    for line in iter {
        let line = line.trim();
        if !line.is_empty() {
            total += line.parse::<i32>().unwrap();
        } else {
            top.push(total);
            total = 0;
            if top.len() > 3 {
                top.sort_unstable_by_key(|k| -k);
                top.resize(3, 0);
            }
        }
    }
    println!("{}", top.iter().sum::<i32>());
}
