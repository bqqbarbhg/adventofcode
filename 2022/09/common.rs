use derive_more::{Neg, Add, AddAssign, Sub, SubAssign, Mul, MulAssign };
use std::cmp;

#[derive(Clone, Copy, PartialEq, Eq, Hash, Neg, Add, AddAssign, Sub, SubAssign, Mul, MulAssign)]
pub struct Vector(pub i32, pub i32);

pub fn clamp<T: Ord>(v: T, min: T, max: T) -> T {
    cmp::min(cmp::max(v, min), max)
}

impl Vector {
    pub fn clamp(v: Vector, min: Vector, max: Vector) -> Vector {
        Vector(clamp(v.0, min.0, max.0), clamp(v.1, min.1, max.1))
    }
    pub fn abs(&self) -> Vector {
        Vector(self.0.abs(), self.1.abs())
    }
    pub fn max(&self) -> i32 {
        cmp::max(self.0, self.1)
    }
}

pub fn parse_move(line: &str) -> Option<(Vector, i32)> {
    let (dir, num) = line.trim().split_once(' ')?;
    let vec = match dir {
        "L" => Vector(-1,  0),
        "R" => Vector( 1,  0),
        "U" => Vector( 0, -1),
        "D" => Vector( 0,  1),
        _ => return None,
    };
    Some((vec, num.parse().ok()?))
}
