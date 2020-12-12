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
    world_pos: Vec2,
    local_pos: Vec2,
    local_x: Vec2,
    local_y: Vec2,
}

fn sindeg(deg: i32) -> i32 { match deg { 0 => 0, 90 => 1, 180 => 0, 270 => -1, _ => panic!() } }
fn cosdeg(deg: i32) -> i32 { match deg { 0 => 1, 90 => 0, 180 => -1, 270 => 0, _ => panic!() } }

fn initial() -> State {
    State {
        world_pos: Vec2 { x: 0, y: 0 },
        local_pos: Vec2 { x: 0, y: 0 },
        local_x: Vec2 { x: 0, y: -1 },
        local_y: Vec2 { x: 1, y: 0 },
    }
}

fn action(s: &str) -> State {
    let (op, n) = s.split_at(1);
    let num: i32 = n.parse().unwrap();
    match op.chars().next().unwrap() {
        'N' => State {
            world_pos: Vec2 { x: 0, y: num },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: 1, y: 0 },
            local_y: Vec2 { x: 0, y: 1 },
        },
        'S' => State {
            world_pos: Vec2 { x: 0, y: -num },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: 1, y: 0 },
            local_y: Vec2 { x: 0, y: 1 },
        },
        'E' => State {
            world_pos: Vec2 { x: num, y: 0 },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: 1, y: 0 },
            local_y: Vec2 { x: 0, y: 1 },
        },
        'W' => State {
            world_pos: Vec2 { x: -num, y: 0 },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: 1, y: 0 },
            local_y: Vec2 { x: 0, y: 1 },
        },
        'L' => State {
            world_pos: Vec2 { x: 0, y: 0 },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: cosdeg(num), y: sindeg(num) },
            local_y: Vec2 { x: -sindeg(num), y: cosdeg(num) },
        },
        'R' => State {
            world_pos: Vec2 { x: 0, y: 0 },
            local_pos: Vec2 { x: 0, y: 0 },
            local_x: Vec2 { x: cosdeg(num), y: -sindeg(num) },
            local_y: Vec2 { x: sindeg(num), y: cosdeg(num) },
        },
        'F' => State {
            world_pos: Vec2 { x: 0, y: 0 },
            local_pos: Vec2 { x: 0, y: num },
            local_x: Vec2 { x: 1, y: 0 },
            local_y: Vec2 { x: 0, y: 1 },
        },
        _ => panic!(),
    }
}

fn concat(a: &State, b: &State) -> State {
    State {
        world_pos: a.world_pos + b.world_pos,
        local_pos: a.local_pos + a.local_x * b.local_pos.x + a.local_y * b.local_pos.y,
        local_x: a.local_x * b.local_x.x + a.local_y * b.local_x.y,
        local_y: a.local_x * b.local_y.x + a.local_y * b.local_y.y,
    }
}

fn position(s: &State) -> Vec2 {
    s.local_pos + s.world_pos
}

fn main() {
    let stdin = io::stdin();

    // Could be parallel! concat() is associative
    let state = stdin.lock().lines()
        .flat_map(|line| line.ok())
        .map(|line| action(&line))
        .fold(initial(), |a, b| concat(&a, &b));

    let pos = position(&state);
    let distance = pos.x.abs() + pos.y.abs();
    println!("{}", distance);
}
