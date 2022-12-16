use std::collections::{VecDeque, HashMap, HashSet};
use regex::Regex;
use std::cmp;

#[derive(Default, Debug)]
pub struct Valve {
    pub name: String,
    pub tunnels: Vec<(usize, u32)>,
    pub flow_rate: u32,
    pub min_route: u32,
}

pub fn parse(lines: impl Iterator<Item=String>) -> Vec<Valve> {
    let mut name_map: HashMap<String, usize> = HashMap::new();
    let mut valves: Vec<Valve> = Vec::new();

    // Could be a lambda but borrow checker can't handle it..
    fn intern(name_map: &mut HashMap<String, usize>, valves: &mut Vec<Valve>, name: String) -> usize {
        let new_index = name_map.len();
        *name_map.entry(name.clone()).or_insert_with(|| {
            valves.push(Valve { name: name, min_route: 1, ..Default::default() });
            new_index
        })
    }

    let re = Regex::new(r"Valve ([A-Z]+) has flow rate=(\d+); tunnels? leads? to valves? ([A-Z]+(, [A-Z]+)*)").unwrap();
    for line in lines.filter(|l| !l.trim().is_empty()) {
        let caps = re.captures(&line).unwrap_or_else(|| panic!("bad line: {}", line));
        let name = caps.get(1).unwrap().as_str();
        let rate: u32 = caps.get(2).unwrap().as_str().parse().unwrap();
        let tunnels = caps.get(3).unwrap().as_str();

        let index = intern(&mut name_map, &mut valves, name.to_owned());
        valves[index].flow_rate = rate;
        valves[index].tunnels = tunnels.split(",")
            .map(|s| intern(&mut name_map, &mut valves, s.trim().to_owned()))
            .map(|ix| (ix, 1))
            .collect();
    }

    valves
}

pub fn simplify(valves: &[Valve]) -> Vec<Valve> {
    fn direct_tunnels(valves: &[Valve], start: usize) -> Vec<(usize, u32)> {
        let mut queue: VecDeque<(usize, u32)> = VecDeque::from([(start, 0)]);
        let mut seen: HashSet<usize> = HashSet::from([start]);
        let mut result: Vec<(usize, u32)> = Vec::new();

        while let Some((index, steps)) = queue.pop_back() {
            if valves[index].flow_rate > 0 && index != start {
                result.push((index, steps));
            }

            for &(next, dist) in &valves[index].tunnels {
                if seen.insert(next) {
                    queue.push_front((next, steps + dist));
                }
            }
        }

        result
    }

    let result: Vec<_> = valves.iter()
        .enumerate()
        .filter(|(_, valve)| valve.flow_rate > 0 || valve.name == "AA")
        .map(|(ix, valve)| Valve {
            name: valve.name.clone(),
            flow_rate: valve.flow_rate,
            tunnels: direct_tunnels(valves, ix),
            min_route: u32::MAX,
        })
        .collect();

    let name_remap: HashMap<_, _> = result.iter()
        .enumerate()
        .map(|(ix, valve)| (valve.name.clone(), ix))
        .collect();

    let mut min_routes: Vec<u32> = vec![u32::MAX; result.len()];
    let mut result: Vec<_> = result.into_iter()
        .map(|mut valve| {
            for tunnel in &mut valve.tunnels {
                tunnel.0 = *name_remap.get(&valves[tunnel.0].name)
                    .unwrap_or_else(|| panic!(
                        "retained removed tunnel {} -> {}",
                        valve.name, valves[tunnel.0].name));
                min_routes[tunnel.0] = cmp::min(min_routes[tunnel.0], tunnel.1);
            }
            valve
        })
        .collect();

    for (ix, valve) in result.iter_mut().enumerate() {
        valve.min_route = min_routes[ix];
    }

    result
}
