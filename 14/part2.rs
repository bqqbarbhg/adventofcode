use std::io::{self,BufRead};
use std::env;

mod shared;

fn simulate(seconds: i32, reindeers: &[shared::Reindeer]) -> Vec<i32> {
	#[derive(Clone, Debug)]
	struct State {
		is_flying: bool,
		time_left: i32,
		position: i32,
		score: i32,
	}

	let initial_state = State {
		is_flying: false,
		time_left: 0,
		position: 0,
		score: 0,
	};

	let mut states = vec![initial_state; reindeers.len()];

	for _ in (0..seconds) {

		let mut max_position = 0i32;

		for (reindeer, state) in reindeers.iter().zip(states.iter_mut()) {
			if state.time_left == 0 {
				if state.is_flying {
					state.is_flying = false;
					state.time_left = reindeer.rest_time;
				} else {
					state.is_flying = true;
					state.time_left = reindeer.fly_time;
				}
			}
			if state.time_left > 0 {
				if state.is_flying {
					state.position += reindeer.fly_speed;
				}
			}

			state.time_left -= 1;
			if state.position > max_position {
				max_position = state.position;
			}
		}

		for state in &mut states {
			if state.position == max_position {
				state.score += 1;
			}
		}
	}

	return states.iter().map(|state| state.score).collect();
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


