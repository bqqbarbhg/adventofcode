use std::{io::{self, BufRead}, cmp};

fn main() {
    let mut max: i32 = 0;
    let mut total: i32 = 0;
    let stdin = io::stdin();
    for line in stdin.lock().lines() {
        let line = line.unwrap();
        let line = line.trim();
        if !line.is_empty() {
            total += line.parse::<i32>().unwrap();
            max = cmp::max(max, total);
        } else {
            total = 0;
        }
    }
    println!("{}", max);
}
