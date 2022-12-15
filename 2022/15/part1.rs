#[macro_use] extern crate scan_fmt;
mod common;
use std::collections::HashSet;
use common::{Point, Bounds, parse};

fn poweset_intersect_imp(row: Point, parent: Bounds, bounds: &[Bounds], count: usize) -> i64 {
    if count == 0 {
        let min_t = (parent.min.0 - row.0).max(parent.min.1 - row.1);
        let max_t = (parent.max.0 - row.0).min(parent.max.1 - row.1);
        return if min_t <= max_t { max_t - min_t + 1 } else { 0 };
    } else if count > bounds.len() {
        return 0;
    }

    let mut sum = 0;
    for (ix, b) in bounds.iter().enumerate() {
        if let Some(isect) = parent.intersect(&b) {
            sum += poweset_intersect_imp(row, isect, &bounds[ix+1..], count - 1);
        }
    }
    sum
}

fn poweset_intersect(row: Point, bounds: &[Bounds], count: usize) -> i64 {
    let mut sum = 0;
    for (ix, &b) in bounds.iter().enumerate() {
        sum += poweset_intersect_imp(row, b, &bounds[ix+1..], count - 1);
    }
    sum
}

fn main() {
    let beacons: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| parse(l.trim()))
        .collect();

    let row_y: i64 = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(2000000);
    let row = Point(0, row_y).rotate();

    let bounds: Vec<_> = beacons.iter().map(|b| b.bounds()).collect();

    let mut total_area = 0;
    for n in 1..beacons.len() {
        let sign = if n%2 == 1 { 1 } else { -1 };
        total_area += sign * poweset_intersect(row, &bounds, n);
    }

    let blocked: HashSet<_> = beacons.iter()
        .flat_map(|b| [b.origin, b.target])
        .collect();

    let on_row = blocked.iter().filter(|b| b.1 == row_y).count();
    println!("{}", total_area - on_row as i64);
}
