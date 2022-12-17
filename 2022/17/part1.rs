mod common;
use common::{Shape, SHAPES};

fn main() {
    let shapes: Vec<_> = SHAPES.iter()
        .map(|s| Shape::parse(s))
        .collect();

    let turns: usize = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(2022);

    let input = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .next()
        .expect("expected one line of input")
        .trim()
        .to_owned();

    let arena_width: u8 = 7;

    let mut arena: Vec<u8> = Vec::new();
    let mut shape_iter = shapes.iter().cycle();
    let mut input_iter = input.chars().cycle();
    for _ in 0..turns {
        let height = arena.len() - arena.iter().rev()
            .position(|&a| a != 0)
            .unwrap_or(arena.len());
        
        let shape = shape_iter.next().unwrap();
        let mut x: u8 = 2;
        let mut y = height + 3;

        let required_height = y + shape.height as usize;
        if required_height > arena.len() {
            arena.resize(required_height, 0);
        }

        loop {
            let new_x = match input_iter.next().unwrap_or('?') {
                '<' => if x > 0 { x - 1 } else { x }
                '>' => if x + shape.width < arena_width { x + 1 } else { x }
                c => panic!("bad input {}", c),
            };

            if !shape.intersect(&arena[y..], new_x) {
                x = new_x;
            }

            if y == 0 || shape.intersect(&arena[y - 1..], x) {
                shape.blit(&mut arena[y..], x);
                break;
            }
            y -= 1;
        }
    }

    let height = arena.len() - arena.iter().rev()
        .position(|&a| a != 0)
        .unwrap_or(arena.len());
    println!("{}", height);
}
