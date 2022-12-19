use derive_more::{Add, AddAssign, Sub, SubAssign, Mul, MulAssign};
use strum_macros::EnumString;
use regex::Regex;
use std::str::FromStr;
use std::cmp::max;

#[derive(Debug, Clone, Copy, EnumString)]
#[strum(ascii_case_insensitive)]
pub enum Resource {
    Ore,
    Clay,
    Obsidian,
    Geode,
}

#[derive(Default, Debug, Clone, Copy, Add, AddAssign, Sub, SubAssign, Mul, MulAssign)]
pub struct Resources {
    pub ore: i32,
    pub clay: i32,
    pub obsidian: i32,
    pub geode: i32,
}

fn div_up(a: i32, b: i32) -> Option<i32> {
    if a > 0 {
        if b > 0 {
            Some((a + b - 1) / b)
        } else {
            None
        }
    } else {
        Some(0)
    }
}

impl Resources {
    pub fn of(r: Resource, amount: i32) -> Resources {
        use Resource::*;
        match r {
            Ore => Resources { ore: amount, ..Default::default() },
            Clay => Resources { clay: amount, ..Default::default() },
            Obsidian => Resources { obsidian: amount, ..Default::default() },
            Geode => Resources { geode: amount, ..Default::default() },
        }
    }

    pub fn div_up(self, other: Resources) -> Option<i32> {
        let ore = div_up(self.ore, other.ore)?;
        let clay = div_up(self.clay, other.clay)?;
        let obsidian = div_up(self.obsidian, other.obsidian)?;
        let geode = div_up(self.geode, other.geode)?;
        Some(max(max(ore, clay), max(obsidian, geode)))
    }
}

#[derive(Debug)]
pub struct Blueprint {
    pub cost: Resources,
    pub output: Resource,
}

pub fn parse_blueprints(line: &str) -> (i32, Vec<Blueprint>) {
    lazy_static! {
        static ref RE_LINE: Regex = Regex::new(r"Blueprint (\d+): (.*)").unwrap();
        static ref RE_ROBOT: Regex = Regex::new(r"Each (\w+) robot costs ([^\.]*)\.").unwrap();
        static ref RE_COST: Regex = Regex::new(r"(\d+) (\w+)").unwrap();
    }

    let caps = RE_LINE.captures(line).unwrap();
    let index: i32 = caps.get(1).unwrap().as_str().parse().unwrap();
    
    let mut robots: Vec<_> = RE_ROBOT.captures_iter(line)
        .map(|caps| {
            let mut cost = Resources::default();
            let output = Resource::from_str(caps.get(1).unwrap().as_str()).unwrap();
            for caps in RE_COST.captures_iter(caps.get(2).unwrap().as_str()) {
                let amount: i32 = caps.get(1).unwrap().as_str().parse().unwrap();
                let resource = Resource::from_str(caps.get(2).unwrap().as_str()).unwrap();
                cost += Resources::of(resource, amount);
            }
            Blueprint { cost: cost, output: output }
        })
        .collect();

    robots.reverse();

    (index, robots)
}

fn simulate_imp(best_seen: &mut Vec<i32>, blueprints: &[Blueprint], time_left: i32, production: Resources, resources: Resources) -> i32 {
    let mut best = resources.geode + production.geode * time_left;
    let potential = best + ((time_left - 1) * time_left) / 2;
    if potential <= best_seen[time_left as usize] {
        return best;
    }

    for bp in blueprints {
        if let Some(wait_time) = (bp.cost - resources).div_up(production) {
            let time = wait_time + 1;
            if time < time_left {
                let res = resources + production * time - bp.cost;
                let prod = production + Resources::of(bp.output, 1);
                let obs = simulate_imp(best_seen, blueprints, time_left - time, prod, res);
                best = max(best, obs);
            }
        }
    }

    let seen = &mut best_seen[time_left as usize];
    *seen = max(*seen, best);

    best
}

pub fn simulate(blueprints: &[Blueprint], time_left: i32) -> i32 {
    let initial_produciton = Resources { ore: 1, ..Default::default() };
    let mut best_seen = vec![0i32; time_left as usize + 1];

    simulate_imp(&mut best_seen, blueprints, time_left, initial_produciton, Resources::default())
}