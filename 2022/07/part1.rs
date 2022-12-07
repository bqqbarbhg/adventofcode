#[macro_use]
extern crate lazy_static;
mod common;
use std::io;
use common::{exec_lines, for_dir_sizes};

fn main() {
    let input = io::stdin().lines().flat_map(|l| l.ok());
    let root = exec_lines(input);

    let mut sum: usize = 0;
    for_dir_sizes(&root, &mut |size| {
        if size <= 100000 {
            sum += size;
        }
    });

    println!("{}", sum);
}
