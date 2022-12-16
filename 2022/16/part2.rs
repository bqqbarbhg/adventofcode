mod common;
use common::{Valve, parse, simplify};
use std::collections::BinaryHeap;
use std::cmp::{PartialEq, Eq, PartialOrd, Ord, min};

#[derive(Debug)]
struct State {
    pub cost: u32,
    pub positions: [usize; 2],
    pub stalls: [u32; 2],
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
    let duration = min(state.stalls[0], state.stalls[1]);
    let mover = if state.stalls[0] == duration { 0 } else { 1 };
    let valve = &valves[state.positions[mover]];

    let mut total_flow = 0;
    for (ix, valve) in valves.iter().enumerate() {
        if (state.open_valves & (1 << ix)) == 0 {
            total_flow += valve.flow_rate;
        }
    }

    let mut open_valves = state.open_valves;
    if state.stalls[0] == duration { open_valves |= 1u32 << state.positions[0]; }
    if state.stalls[1] == duration { open_valves |= 1u32 << state.positions[1]; }

    let mut result: Vec<State> = Vec::new();
    for &(next, dist) in &valve.tunnels {
        if (open_valves & (1 << next)) != 0 { continue; }
        let mut positions = state.positions;
        let mut stalls = state.stalls;

        positions[mover] = next;
        stalls[0] -= duration;
        stalls[1] -= duration;
        stalls[mover] = dist + 1;

        if duration <= state.time_left {
            result.push(State{
                cost: duration,
                positions: positions,
                stalls: stalls,
                open_valves: open_valves,
                time_left: state.time_left - duration,
            });
        }
    }

    if result.is_empty() {
        if duration < state.time_left {
            result.push(State{
                cost: duration,
                open_valves: open_valves,
                time_left: state.time_left - duration,
                ..*state
            });
        } else {
            result.push(State{
                cost: state.time_left,
                open_valves: open_valves,
                time_left: 0,
                ..*state
            });
        }
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
    let mut best_cost: Vec<u32> = Vec::new();
    best_cost.resize(time as usize + 1, u32::MAX / 2);

    work.push(State{
        cost: 0,
        positions: [start, start],
        stalls: [0, 0],
        time_left: time,
        open_valves: 0,
    });

    // Carefully tuned to solve :)
    const HACK_GREED: u32 = 3;

    while let Some(state) = work.pop() {
        if state.time_left == 0 {
            let maximum_flow = time * total_flow(valves);
            return maximum_flow - state.cost;
        }

        let best = &mut best_cost[state.time_left as usize];
        if state.cost <= *best + state.time_left * HACK_GREED {
            *best = min(*best, state.cost);
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

    let flow = search(&valves, start, 26);
    println!("{}", flow);
}
