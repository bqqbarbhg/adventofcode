use std::io::{self, Read};

fn main() {

    // Split input into numbers representing crab positions and sort them.
    let mut input = String::new();
    io::stdin().read_to_string(&mut input).unwrap();
    let mut positions: Vec<i32> = input
        .split(",")
        .flat_map(|line| line.parse().ok())
        .collect();
    let num = positions.len() as i32;
    positions.sort();

    #[derive(Clone)]
    struct Split {
        pos: i32,
        cost: i32,
    }

    // Calculate the cost if we would select the first position.
    let first_pos = positions[0];
    let first_cost: i32 = positions.iter().map(|x| x - first_pos).sum();
    let first_split = Split {
        pos: first_pos,
        cost: first_cost,
    };

    // Scan the positions and incrementally update the cost.
    // On each step we move the target position right by `delta`. As the cost
    // is linear we increase it by `delta` for all the crabs on the left and
    // decrease it by `delta` for all the ones on the right.
    let best_split = positions.iter()
        .enumerate()
        .scan(first_split, |split, (ix, &pos)| {
            let delta = pos - split.pos;
            let num_left = ix as i32;
            let num_right = num - num_left;
            split.cost = split.cost + (num_left - num_right) * delta;
            split.pos = pos;
            Some(split.clone())
        })
        .min_by_key(|split| split.cost)
        .unwrap();

    // Output the resulting cost, we also have the position in `best_split.pos`.
    println!("{}", best_split.cost);
}