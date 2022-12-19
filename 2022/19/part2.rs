#[macro_use]
extern crate lazy_static;
extern crate derive_more;
mod common;
use common::{parse_blueprints, simulate};
use rayon::prelude::*;

fn main() {
    let blueprints: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| parse_blueprints(&l))
        .take(3)
        .collect();

    let result: i32 = blueprints.par_iter()
        .map(|(_, bp)| simulate(bp, 32))
        .product();
    
    println!("{}", result);
}

