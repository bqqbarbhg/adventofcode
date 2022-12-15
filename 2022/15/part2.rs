#[macro_use] extern crate scan_fmt;
mod common;
use common::{Point, Beacon, parse};
use rayon::prelude::*;

fn beacon_sdf(beacon: &Beacon, p: Point) -> i64 {
    let (dx, dy) = (p.0 - beacon.origin.0, p.1 - beacon.origin.1);
    dx.abs() + dy.abs() - (beacon.distance + 1)
}

fn edge_sdf(extent: i64, p: Point) -> i64 {
    (p.0).min(p.1).min(extent - p.0).min(extent - p.1)
}

fn scene_sdf(beacons: &[Beacon], extent: i64, p: Point) -> i64 {
    let d = beacons.iter().map(|b| beacon_sdf(b, p)).min().unwrap();
    d.min(edge_sdf(extent, p))
}

static DIRS: [Point; 8] = [
    Point(-1, 0), Point(1, 0), Point(0, -1), Point(0, 1),
    Point(-1, -1), Point(1, -1), Point(-1, 1), Point(1, 1),
];

fn step<F: Fn(Point) -> i64>(sdf: &F, pos: Point) -> Option<(Point, i64)> {
    let d = sdf(pos);
    if d >= 0 { return Some((pos, 0)) }

    let mut step = d;
    loop {
        for dir in DIRS {
            let p = Point(pos.0 + dir.0 * step, pos.1 + dir.1 * step);
            let det = dir.0.abs() + dir.1.abs();
            let d2 = sdf(p);
            if d2 > d && d2 - d >= det * step {
                return Some((p, d2));
            }
        }

        if step == 1 { break; }
        step = 1.max(step / 2);
    }

    None
}

fn walk<F: Fn(Point) -> i64>(sdf: &F, start: Point) -> Option<Point> {
    let mut pos = start;
    loop {
        match step(sdf, pos) {
            Some((p, 0)) => { return Some(p) },
            Some((p, _)) => { pos = p; }
            None => { return None },
        }
    }
}

fn main() {
    let beacons: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| parse(l.trim()))
        .collect();

    let extent: i64 = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(4000000);

    let sdf = |p| scene_sdf(&beacons, extent, p);

    let found = beacons.iter()
        .flat_map(|b| DIRS.map(|d| (b,d)))
        .par_bridge()
        .map(|(beacon, dir)| {
            let det = dir.0.abs() + dir.1.abs();
            let step = beacon.distance / det;
            let start = Point(beacon.origin.0 + dir.0 * step, beacon.origin.1 + dir.1 * step);
            if let Some(p) = walk(&sdf, start) {
                return Some(p)
            } else {
                None
            }
        })
        .find_any(|p| p.is_some());

    let point = found.expect("could not find point").unwrap();
    println!("{}", point.0*4000000 + point.1);
}
