mod common;
use common::{Valve, parse, simplify};
use std::collections::{HashSet, BinaryHeap};
use std::cmp::{PartialEq, Eq, PartialOrd, Ord};

#[derive(Debug)]
struct State {
    pub cost: u32,
    pub position: usize,
    pub open_valves: u32,
    pub time_left: u32,
}

impl Eq for State { }
impl PartialEq for State {
    fn eq(&self, other: &Self) -> bool { self.cost.eq(&other.cost) }
}
impl Ord for State {
    fn cmp(&self, other: &Self) -> std::cmp::Ordering { self.cost.cmp(&other.cost).reverse() }
}
impl PartialOrd for State {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> { Some(self.cmp(&other)) }
}

fn actions(valves: &[Valve], state: &State) -> Vec<State> {
    let valve = &valves[state.position];
    let mut result: Vec<State> = Vec::new();

    let mut total_flow = 0;
    for (ix, valve) in valves.iter().enumerate() {
        if (state.open_valves & (1 << ix)) == 0 {
            total_flow += valve.flow_rate;
        }
    }

    for &(next, dist) in &valve.tunnels {
        if (state.open_valves & (1 << next)) != 0 { continue; }

        let duration = dist + 1;
        if state.time_left >= duration {
            result.push(State{
                cost: duration,
                position: next,
                open_valves: state.open_valves | 1u32 << next,
                time_left: state.time_left - duration,
            });
        }
    }

    if result.is_empty() {
        result.push(State{
            cost: state.time_left,
            position: state.position,
            open_valves: state.open_valves,
            time_left: 0,
        });
    }

    for next in &mut result {
        next.cost = state.cost + next.cost * total_flow;
    }

    result
}

fn total_flow(valves: &[Valve]) -> u32 {
    valves.iter().map(|v| v.flow_rate).sum()
}

fn search(valves: &[Valve], start: usize, time: u32) -> u32 {
    let mut work: BinaryHeap<State> = BinaryHeap::new();
    let mut closed: HashSet<(usize, u32)> = HashSet::new();

    work.push(State{
        cost: 0,
        position: start,
        time_left: time,
        open_valves: 0,
    });

    while let Some(state) = work.pop() {
        if state.time_left == 0 {
            let maximum_flow = time * total_flow(valves);
            return maximum_flow - state.cost;
        }
        if closed.insert((state.position, state.open_valves)) {
            work.extend(actions(valves, &state));
        }
    }

    unreachable!();
}

fn main() {
    let valves = parse(std::io::stdin().lines().flat_map(|l| l.ok()));
    let valves = simplify(&valves);

    let start = valves.iter().position(|v| v.name == "AA")
        .expect("could not find start 'AA'");

    let flow = search(&valves, start, 30);
    println!("{}", flow);
}
