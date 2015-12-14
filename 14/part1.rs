use std::io::{self,BufRead};
use std::env;

mod shared;

fn simulate(seconds: i32, reindeers: &[shared::Reindeer]) -> Vec<i32> {
	#[derive(Clone, Debug)]
	struct State {
		is_flying: bool,
		position: i32,
		prev_update: i32,
		next_update: i32,
	}

	let initial_state = State {
		is_flying: false,
		position: 0,
		prev_update: 0,
		next_update: 0,
	};

	let mut time = 0i32;
	let mut states = vec![initial_state; reindeers.len()];

	while time < seconds {

		let mut next_time = i32::max_value();

		for (reindeer, state) in reindeers.iter().zip(states.iter_mut()) {

			if state.next_update == time {
				if state.is_flying {
					state.is_flying = false;
					state.next_update = time + reindeer.rest_time;
					state.position += (time - state.prev_update) * reindeer.fly_speed;
				} else {
					state.is_flying = true;
					state.next_update = time + reindeer.fly_time;
				}
				state.prev_update = time;
			}

			if next_time > state.next_update {
				next_time = state.next_update
			}
		}

		time = next_time;
	}

	for (reindeer, state) in reindeers.iter().zip(states.iter_mut()) {
		if state.is_flying {
			state.position += (seconds - state.prev_update) * reindeer.fly_speed;
		}
	}

	return states.iter().map(|state| state.position).collect();
}

fn main() {
	let stdin = io::stdin();
	let reindeers: Vec<shared::Reindeer> = stdin.lock().lines()
		.filter_map(Result::ok)
		.filter_map(|s| shared::Reindeer::from_line(&s))
		.collect();

	let ticks = match env::args().nth(1) {
		Some(ticks_str) => match ticks_str.parse() {
			Ok(ticks) => ticks,
			Err(error) => panic!("Failed to parse simulation time: {}", error),
		},
		None => panic!("No simulation time argument given"),
	};

	let positions = simulate(ticks, &reindeers);

	if let Some(max_position) = positions.iter().max() {
		println!("{}", max_position)
	} else {
		panic!("Simulation exited with no reindeers")
	}
}

