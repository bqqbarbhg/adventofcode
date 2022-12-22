#![allow(dead_code)]
use derive_more::{Add, Sub, Neg};
use bit_vec::BitVec;
use regex::Regex;
use std::cmp::{PartialEq, Eq};
use std::hash::Hash;

#[derive(Debug)]
pub struct Map {
    pub width: i32,
    pub height: i32,
    pub ground: BitVec,
    pub blocked: BitVec,
    pub start: Vector,
}

pub type Dir = u8;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash, Add, Sub, Neg)]
pub struct Vector(pub i32, pub i32);

impl Vector {
    pub fn turn(self, dir: u8) -> Vector {
        let Vector(x, y) = self;
        match dir {
            0 => Vector(x, y),
            1 => Vector(-y, x),
            2 => Vector(-x, -y),
            3 => Vector(y, -x),
            _ => unreachable!(),
        }
    }
    pub fn face(self, dir: u8) -> Vector {
        let Vector(x, y) = self;
        match dir {
            0 => Vector(x, y),
            1 => Vector(y, x),
            2 => Vector(-x, y),
            3 => Vector(-y, x),
            _ => unreachable!(),
        }
    }
    pub fn unface(self, dir: u8) -> Vector {
        let Vector(x, y) = self;
        match dir {
            0 => Vector(x, y),
            1 => Vector(y, x),
            2 => Vector(-x, y),
            3 => Vector(y, -x),
            _ => unreachable!(),
        }
    }
}

impl Map {
    fn to_linear(&self, Vector(x, y): Vector) -> Option<usize> {
        if x >= 0 && y >= 0 && x < self.width && y < self.height {
            Some(y as usize * self.width as usize + x as usize)
        } else {
            None
        }
    }

    pub fn is_ground(&self, pos: Vector) -> bool {
        self.to_linear(pos).map(|ix| self.ground.get(ix).unwrap()).unwrap_or(false)
    }

    pub fn is_blocked(&self, pos: Vector) -> bool {
        self.to_linear(pos).map(|ix| self.blocked.get(ix).unwrap()).unwrap_or(false)
    }

    pub fn parse(lines: impl Iterator<Item=String>) -> Map {
        let mut result: Vec<Vec<u8>> = lines
            .map(|line| {
                line.trim_end().chars()
                    .map(|ch| match ch {
                        ' ' => 0,
                        '.' => 1,
                        '#' => 2,
                        _ => panic!("bad character {}", ch),
                    })
                    .collect()
            })
            .collect();

        let height = result.len();
        let width = result.iter().map(|v| v.len()).max().unwrap();
        for row in &mut result {
            row.resize(width, 0);
        }

        let ground: BitVec = result.iter()
            .flat_map(|row| row.iter().map(|&b| b != 0))
            .collect();
        let blocked: BitVec = result.iter()
            .flat_map(|row| row.iter().map(|&b| b == 2))
            .collect();

        let start_x = result[0].iter().position(|&b| b != 0)
            .expect("could not find start position on first row");

        Map {
            width: width as i32,
            height: height as i32,
            ground: ground,
            blocked: blocked,
            start: Vector(start_x as i32, 0),
        }
    }
}

#[derive(Debug)]
pub enum Move {
    Forward(i32),
    Turn(u8),
}

impl Move {
    pub fn parse(line: &str) -> Vec<Move> {
        let re = Regex::new(r"(\d+)|([LR])").unwrap();
        re.captures_iter(line)
            .map(|caps| {
                let c1 = caps.get(1).map(|c| c.as_str());
                let c2 = caps.get(2).map(|c| c.as_str());
                match (c1, c2) {
                    (Some(num), None) => Move::Forward(num.parse().unwrap()),
                    (None, Some("R")) => Move::Turn(1),
                    (None, Some("L")) => Move::Turn(3),
                    _ => unreachable!(),
                }
            })
            .collect()
    }
}

pub trait Topology {
    fn wrap(&self, pos: Vector, dir: Dir) -> (Vector, Dir);
}

pub fn walk(map: &Map, topo: &impl Topology, moves: &[Move]) -> (Vector, Dir) {
    let mut dir = 0u8;
    let mut pos = map.start;

    for m in moves {
        match m {
            Move::Turn(d) => {
                dir = (dir + d) % 4;
            },
            Move::Forward(steps) => {
                for _ in 0..*steps {
                    let d = Vector(1, 0).turn(dir);
                    let pre_wrap = Vector(pos.0 + d.0, pos.1 + d.1);
                    let (next_pos, next_dir) = topo.wrap(pre_wrap, dir);
                    if !map.is_blocked(next_pos) {
                        pos = next_pos;
                        dir = next_dir;
                    }
                }
            },
        }
    }

    (pos, dir)
}
