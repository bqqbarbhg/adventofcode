extern crate derive_more;
mod common;

use common::{Vector, parse_move};
use std::collections::HashSet;

const ONE: Vector = Vector(1, 1);

fn main() {
    let moves = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| parse_move(&l));

    let mut seen: HashSet<Vector> = HashSet::new();
    let mut head = Vector(0, 0);
    let mut tail = Vector(0, 0);
    for (dir, count) in moves {
        for _ in 0..count {
            head += dir;
            let delta = head - tail;
            if delta.abs().max() > 1 {
                tail += Vector::clamp(delta, -ONE, ONE);
            }
            seen.insert(tail);
        }
    }

    println!("{}", seen.len());
}
