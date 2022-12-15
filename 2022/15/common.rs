#![allow(dead_code)]
use std::cmp::{PartialEq, Eq};
use std::hash::Hash;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
pub struct Point(pub i64, pub i64);

impl Point {
    pub fn min(&self, other: &Point) -> Point {
        Point(self.0.min(other.0), self.1.min(other.1))
    }
    pub fn max(&self, other: &Point) -> Point {
        Point(self.0.max(other.0), self.1.max(other.1))
    }
    pub fn rotate(&self) -> Point {
        Point(self.0 - self.1, self.0 + self.1)
    }
    pub fn unrotate(&self) -> Point {
        Point((self.0 + self.1) / 2, (self.1 - self.0) / 2)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Bounds {
    pub min: Point,
    pub max: Point,
}

impl Bounds {
    pub fn intersect(&self, other: &Bounds) -> Option<Bounds> {
        let min = self.min.max(&other.min);
        let max = self.max.min(&other.max);
        if min.0 <= max.0 && min.1 <= max.1 {
            Some(Bounds{ min: min, max: max })
        } else {
            None
        }
    }

    pub fn original_area(&self) -> i64 {
        let (dx, dy) = (self.max.0 - self.min.0, self.max.1 - self.min.1);
        let is_even = self.min.0 + self.min.1 + 1;
        
        // We are counting how many even (`(x+y)%2 == 0`) squares are contained
        // within the bounds. For even sizes this is simply half of the squares
        // but for odd we can have two configurations:
        //     |# # #|    | # # |
        //     | # # |    |# # #|
        //     |# # #|    | # # |
        // Here we can round based on the parity of the top-left corner, rounding
        // up if even (left figure) and down if odd (right figure).
        ((dx + 1)*(dy + 1) + (is_even & 1)) / 2
    }
}

#[derive(Debug)]
pub struct Beacon {
    pub origin: Point,
    pub target: Point,
    pub distance: i64,
}

pub fn parse(line: &str) -> Beacon {
    let (x0, y0, x1, y1) = scan_fmt!(line,
        "Sensor at x={}, y={}: closest beacon is at x={}, y={}",
        i64, i64, i64, i64).unwrap();
    let d = (x1 - x0).abs() + (y1 - y0).abs();
    Beacon { origin: Point(x0, y0), target: Point(x1, y1), distance: d }
}

impl Beacon {
    pub fn bounds(&self) -> Bounds {
        let Point(x0, y0) = self.origin;
        Bounds {
            min: Point(x0 - self.distance, y0).rotate(),
            max: Point(x0 + self.distance, y0).rotate(),
        }
    }
}
