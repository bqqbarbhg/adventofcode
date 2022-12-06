mod common;
use std::{io::{self, BufRead}};
use std::vec::Vec;
use regex::Regex;
use common::{match_iter, match_capture, ensure_size};

type Action = (i32, usize, usize);

fn main() {
    let re_top = Regex::new(r" {4}|\[([A-Z])\]").unwrap();
    let re_action = Regex::new(r"move (\d+) from (\d+) to (\d+)").unwrap();
    let mut stacks: Vec<Vec<char>> = Vec::new();

    let mut input = io::stdin().lock().lines()
        .flat_map(|l| l.ok());

    for line in &mut input {
        if line.trim().is_empty() { break }
        for (ix, ch) in match_iter::<char>(&re_top, &line).enumerate() {
            if let Some(ch) = ch {
                ensure_size(&mut stacks, ix + 1);
                stacks[ix].push(ch);
            }
        }
    }

    for stack in &mut stacks {
        stack.reverse()
    }

    for line in input {
        if let Some((num, src, dst)) = match_capture::<Action>(&re_action, &line) {
            for _ in 0..num {
                let item = stacks[src - 1].pop().expect("Invalid move");
                stacks[dst - 1].push(item);
            }
        }
    }

    let letters = stacks.iter().map(|s| s.last().expect("Stack is empty"));
    println!("{}", letters.collect::<String>());
}
