mod common;
use common::{Shape, SHAPES};
use std::collections::HashMap;

fn main() {
    let shapes: Vec<_> = SHAPES.iter()
        .map(|s| Shape::parse(s))
        .collect();

    let turns: usize = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(1000000000000);

    let input = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .next()
        .expect("expected one line of input")
        .trim()
        .to_owned();

    let arena_width: u8 = 7;

    let mut arena: Vec<u8> = Vec::new();

    let mut shape_iter = shapes.iter().enumerate().cycle();
    let mut input_iter = input.chars().enumerate().cycle();

    let mut seen: HashMap<(u8, u32, u64), (usize, usize)> = HashMap::new();
    let mut tracing: bool = true;

    let mut full_line = 0;
    let mut turn = 0;
    let mut skip_height = 0;
    while turn < turns {
        let height = arena.len() - arena.iter().rev()
            .position(|&a| a != 0)
            .unwrap_or(arena.len());
        
        let (shape_ix, shape) = shape_iter.next().unwrap();
        let start_y = height + 3;

        let required_height = start_y + shape.height as usize;
        if required_height > arena.len() {
            arena.resize(required_height, 0);
        }

        let mut x: u8 = 2;
        let mut y = start_y;
        loop {
            let (input_ix, input) = input_iter.next().unwrap();
            let new_x = match input {
                '<' => if x > 0 { x - 1 } else { x },
                '>' => if x + shape.width < arena_width { x + 1 } else { x },
                c => panic!("bad input {}", c),
            };

            if !shape.intersect(&arena[y..], new_x) {
                x = new_x;
            }

            if y == 0 || shape.intersect(&arena[y - 1..], x) {
                let trace_height = height - y + 1;
                if tracing && y >= 8 && trace_height <= 8 {
                    let mut trace: u64 = 0;
                    let mut trace_line: u8 = 0;
                    for i in 0..8 {
                        let line = arena[y + trace_height - i - 1];
                        trace_line |= line;
                        trace |= (line as u64) << (i * 4);
                        if i >= trace_height && trace_line == full_line {
                            break;
                        }
                    }

                    if trace_line == full_line {
                        let entry = (shape_ix as u8, input_ix as u32, trace);
                        if let Some((prev_turn, prev_y)) = seen.insert(entry, (turn, y)) {
                            let cycle_turns = turn - prev_turn;
                            let cycle_height = y - prev_y;

                            let cycle_count = (turns - turn) / cycle_turns;
                            turn += cycle_count * cycle_turns;
                            skip_height += cycle_count * cycle_height;

                            tracing = false;
                        }
                    }
                }

                shape.blit(&mut arena[y..], x);
                for line in &arena[y..] {
                    full_line |= line;
                }

                break;
            }
            y -= 1;
        }
    
        turn += 1;
    }

    let height = arena.len() - arena.iter().rev()
        .position(|&a| a != 0)
        .unwrap_or(arena.len());
    println!("{}", height + skip_height);
}
