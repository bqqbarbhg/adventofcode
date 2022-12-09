extern crate derive_more;
mod common;

use common::{Vector, parse_move};
use std::collections::HashSet;

const ONE: Vector = Vector(1, 1);

fn main() {
    let moves = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| parse_move(&l));

    let length: usize = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(10);

    let mut seen: HashSet<Vector> = HashSet::new();
    let mut nodes: Vec<Vector> = vec![Vector(0, 0); length];
    for (dir, count) in moves {
        for _ in 0..count {
            nodes[0] += dir;
            for i in 1..nodes.len() {
                let delta = nodes[i - 1] - nodes[i];
                if delta.abs().max() > 1 {
                    nodes[i] += Vector::clamp(delta, -ONE, ONE);
                }
            }
            seen.insert(*nodes.last().unwrap());
        }
    }

    println!("{}", seen.len());
}
