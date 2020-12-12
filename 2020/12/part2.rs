use std::io::{self, BufRead};
use std::ops;

#[derive(Clone, Copy, Debug)]
struct Vec2 {
    x: i32,
    y: i32,
}

impl ops::Add<Vec2> for Vec2 {
    type Output = Vec2;
    fn add(self, rhs: Vec2) -> Vec2 { Vec2 { x: self.x + rhs.x, y: self.y + rhs.y } }
}

impl ops::Mul<i32> for Vec2 {
    type Output = Vec2;
    fn mul(self, rhs: i32) -> Vec2 { Vec2 { x: self.x * rhs, y: self.y * rhs } }
}

#[derive(Debug)]
struct State {
    ship_pos: Vec2,
    target_pos: Vec2,
}

fn sindeg(deg: i32) -> i32 { match deg { 0 => 0, 90 => 1, 180 => 0, 270 => -1, _ => panic!() } }
fn cosdeg(deg: i32) -> i32 { match deg { 0 => 1, 90 => 0, 180 => -1, 270 => 0, _ => panic!() } }

fn initial() -> State {
    State {
        ship_pos: Vec2 { x: 0, y: 0 },
        target_pos: Vec2 { x: 10, y: 1 },
    }
}

fn update(s: &State, line: &str) -> State {
    let (op, n) = line.split_at(1);
    let num: i32 = n.parse().unwrap();
    match op.chars().next().unwrap() {
        'N' => State {
            ship_pos: s.ship_pos,
            target_pos: s.target_pos + Vec2 { x: 0, y: num },
        },
        'S' => State {
            ship_pos: s.ship_pos,
            target_pos: s.target_pos + Vec2 { x: 0, y: -num },
        },
        'E' => State {
            ship_pos: s.ship_pos,
            target_pos: s.target_pos + Vec2 { x: num, y: 0 },
        },
        'W' => State {
            ship_pos: s.ship_pos,
            target_pos: s.target_pos + Vec2 { x: -num, y: 0 },
        },
        'L' => State {
            ship_pos: s.ship_pos,
            target_pos: Vec2 { x: cosdeg(num), y: sindeg(num) } * s.target_pos.x
                + Vec2 { x: -sindeg(num), y: cosdeg(num) } * s.target_pos.y,
        },
        'R' => State {
            ship_pos: s.ship_pos,
            target_pos: Vec2 { x: cosdeg(num), y: -sindeg(num) } * s.target_pos.x
                + Vec2 { x: sindeg(num), y: cosdeg(num) } * s.target_pos.y,
        },
        'F' => State {
            ship_pos: s.ship_pos + s.target_pos * num,
            target_pos: s.target_pos,
        },
        _ => panic!(),
    }
}

fn main() {
    let stdin = io::stdin();

    // Could be parallel!
    let state = stdin.lock().lines()
        .flat_map(|line| line.ok())
        .fold(initial(), |s, line| update(&s, &line));

    let pos = state.ship_pos;
    let distance = pos.x.abs() + pos.y.abs();
    println!("{}", distance);
}
