use fxhash::FxHashSet;
use std::ops::RangeInclusive;

pub type Point = (i32, i32);

fn signed_range(a: i32, b: i32) -> RangeInclusive<i32> {
    if a < b { a..=b } else { b..=a }
}

pub fn parse_line(dst: &mut FxHashSet<Point>, line: &str) {
    let mut prev: Option<Point> = None;

    for part in line.split("->") {
        let (sx, sy) = part.trim().split_once(',')
            .expect("expected x,y coordinate");
        let point: Point = (
            sx.parse().expect("invalid x"),
            sy.parse().expect("invalid y"));

        if let Some(prev) = prev {
            for x in signed_range(point.0, prev.0) {
                for y in signed_range(point.1, prev.1) {
                    dst.insert((x, y));
                }
            }
        }
        prev = Some(point);
    }
}

pub fn parse_lines(lines: impl Iterator<Item=String>) -> FxHashSet<Point> {
    let mut result = FxHashSet::default();
    for line in lines {
        parse_line(&mut result, &line);
    }
    result
}
