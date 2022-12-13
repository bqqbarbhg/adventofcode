use std::collections::{BinaryHeap, HashSet};
use std::hash::Hash;
use std::cmp::{PartialEq, PartialOrd, Ord, Ordering};
use std::rc::Rc;

#[derive(Clone)]
struct Path<S>(pub S, pub Option<Rc<Self>>);
struct Entry<S>(pub u64, pub u64, pub Path<S>);
impl<S> Eq for Entry<S> { }
impl<S> PartialEq for Entry<S> {
    fn eq(&self, other: &Self) -> bool { self.0.eq(&other.0) }
}
impl<S> PartialOrd for Entry<S> {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        self.0.partial_cmp(&other.0).map(Ordering::reverse)
    }
}
impl<S> Ord for Entry<S> {
    fn cmp(&self, other: &Self) -> Ordering {
        self.0.cmp(&other.0).reverse()
    }
}

pub struct StateNext<S> {
    pub state: S,
    pub cost: u64,
    pub heuristic: u64,
}

pub enum StateInfo<S> {
    Node(Vec<StateNext<S>>),
    Goal,
}

pub fn search<S : Sized + Eq + Clone + Hash, F: FnMut(S) -> StateInfo<S>>(start: S, mut f: F) -> Option<Vec<S>> {
    let mut closed: HashSet<S> = HashSet::new();    
    let mut work: BinaryHeap<Entry<S>> = BinaryHeap::new();
    work.push(Entry(0, 0, Path(start.clone(), None)));

    while !work.is_empty() {
        let Entry(_, score, path) = work.pop().unwrap();
        let state = &path.0;
        if !closed.insert(state.clone()) { continue; }

        match f(state.clone()) {
            StateInfo::Node(children) => {
                for next in children {
                    if closed.contains(&next.state) { continue; }
                    let path = Path(next.state, Some(Rc::new(path.clone())));
                    work.push(Entry(score + next.cost + next.heuristic, score + next.cost, path));
                }
            },
            StateInfo::Goal => {
                let mut linear_path: Vec<S> = Vec::new();
                let mut p = &path;
                loop {
                    linear_path.push(p.0.clone());
                    p = match &p.1 {
                        Some(next) => next.as_ref(),
                        None => { break; }
                    }
                }
                linear_path.reverse();
                return Some(linear_path);
            },
        }
    }

    None
}

#[derive(Clone, Copy, PartialEq, Eq, Hash)]
pub struct Vector(pub i32, pub i32);

pub struct Grid {
    pub values: Vec<i32>,
    pub width: i32,
    pub height: i32,
}

impl Grid {
    pub fn get(&self, Vector(x, y): Vector) -> Option<i32> {
        if x < 0 || y < 0 || x >= self.width || y >= self.height {
            return None
        }
        Some(self.values[(y * self.width + x) as usize])
    }
}

pub struct Scene {
    pub grid: Grid,
    pub start: Vector,
    pub end: Vector,
}

pub fn parse_scene(lines: impl Iterator<Item=String>) -> Scene {
    let mut data: Vec<i32> = Vec::new();
    let mut start: Option<Vector> = None;
    let mut end: Option<Vector> = None;
    let mut height = 0;
    let mut width = 0;

    for (y, line) in lines.enumerate() {
        let mut line = line.trim().to_owned();
        if y == 0 {
            width = line.len();
        } else {
            assert_eq!(line.len(), width);
        }
        height = y + 1;

        if let Some(x) = line.find('S') {
            line = line.replace('S', "a");
            start = Some(Vector(x as i32, y as i32));
        }
        if let Some(x) = line.find('E') {
            line = line.replace('E', "z");
            end = Some(Vector(x as i32, y as i32));
        }
        data.extend(line.as_bytes().iter().map(|b| *b as i32 - 'a' as i32));
    }

    Scene {
        grid: Grid {
            values: data,
            width: width as i32,
            height: height as i32,
        },
        start: start.expect("did not find start"),
        end: end.expect("did not find end"),
    }
}
