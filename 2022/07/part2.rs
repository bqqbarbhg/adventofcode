#[macro_use]
extern crate lazy_static;
mod common;
use std::io;
use common::{exec_lines, for_dir_sizes};

fn main() {
    let input = io::stdin().lines().flat_map(|l| l.ok());
    let root = exec_lines(input);

    let capacity = 70000000usize;
    let needs_unused = 30000000usize;
    let total_size = for_dir_sizes(&root, &mut |_|());

    let mut max: usize = usize::MAX;
    for_dir_sizes(&root, &mut |size| {
        if total_size - size <= capacity - needs_unused {
            max = max.min(size);
        }
    });

    println!("{}", max);
}
