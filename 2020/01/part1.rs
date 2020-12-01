use std::io::{self, BufRead};
use std::collections::HashSet;

fn main() {
    let stdin = io::stdin();
    let goal = 2020;

    let input = stdin.lock().lines()
        .flat_map(|line| line.ok())
        .flat_map(|line| line.parse::<u32>())
        .filter(|num| num <= &goal);

    let mut seen = HashSet::<u32>::new();
    for num in input {
        let other = goal - num;
        if seen.contains(&other) {
            println!("{}", num * other);
            break;
        }
        seen.insert(num);
    }
}
