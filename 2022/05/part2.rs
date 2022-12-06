mod common;
use std::{io::{self, BufRead}};
use std::vec::Vec;
use regex::Regex;
use common::{match_iter, match_capture, ensure_size};

fn get_disjoint<T>(ts: &mut [T], a: usize, b: usize) -> (&mut T, &mut T) {
    assert!(a != b, "a ({}) and b ({}) must be disjoint", a, b);
    assert!(a < ts.len(), "a ({}) is out of bounds", a);
    assert!(b < ts.len(), "b ({}) is out of bounds", b);
    if a < b {
        let (al, bl) = ts.split_at_mut(b);
        (&mut al[a], &mut bl[0])
    } else {
        let (bl, al) = ts.split_at_mut(a);
        (&mut al[0], &mut bl[b])
    }
}

type Action = (usize, usize, usize);

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
            let (src, dst) = get_disjoint(&mut stacks, src - 1, dst - 1);
            assert!(num <= src.len(), "Trying to pop {} from {}", num, src.len());
            let pos = src.len() - num;
            dst.extend(&src[pos..]);
            src.truncate(pos);
        }
    }

    let letters = stacks.iter().map(|s| s.last().expect("Stack is empty"));
    println!("{}", letters.collect::<String>());
}
