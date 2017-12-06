use std::io::{self, Read};
use std::collections::HashSet;
use std::iter::Iterator;

fn find_bank<'a, I>(banks: I) -> Option<usize>
    where I: IntoIterator<Item = &'a i32> {

    let mut iter = banks.into_iter();
    match iter.next() {
        Some(first_size) => {
            let mut best_index = 0;
            let mut best_size = first_size;
            for (index, size) in iter.enumerate() {
                if size > best_size {
                    best_index = index + 1;
                    best_size = size;
                }
            }
            Some(best_index)
        }
        None => None
    }
}

fn redistribute(banks: &mut Vec<i32>) {
    let index = find_bank(banks.iter()).unwrap();
    let num = banks[index];
    let len = banks.len();
    banks[index] = 0;
    for offset in 1 .. num+1 {
        banks[(index + offset as usize) % len] += 1
    }
}

fn main() {
    let mut input = String::new();
    io::stdin().read_to_string(&mut input).unwrap();
    let mut banks: Vec<i32> = input
        .split(char::is_whitespace)
        .flat_map(|line| line.parse().ok())
        .collect();

    let mut seen = HashSet::new();
    while !seen.contains(&banks) {
        seen.insert(banks.clone());
        redistribute(&mut banks);
    }

    let mut cycles = 1;
    let cycle_start = banks.clone();
    redistribute(&mut banks);

    while banks != cycle_start {
        cycles += 1;
        redistribute(&mut banks);
    }

    println!("{}", cycles);
}


