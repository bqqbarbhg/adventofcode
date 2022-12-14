mod common;
use common::{Point, parse_lines};
use fxhash::FxHashSet;

fn simulate(rock: &FxHashSet<Point>, floor: i32, mut sand: Point) -> Option<Point> {
    if rock.contains(&sand) { return None; }
    loop {
        let (x, y) = sand;
        if y >= floor {
            return Some(sand);
        } else if !rock.contains(&(x, y + 1)) {
            sand = (x, y + 1);
        } else if !rock.contains(&(x - 1, y + 1)) {
            sand = (x - 1, y + 1);
        } else if !rock.contains(&(x + 1, y + 1)) {
            sand = (x + 1, y + 1);
        } else {
            return Some(sand);
        }
    }
}

fn main() {
    let mut rock = parse_lines(std::io::stdin().lines().flat_map(|l| l.ok()));
    let floor = rock.iter().map(|(_, y)| y).max().unwrap() + 1;

    let mut num_turns = 0;
    loop {
        match simulate(&rock, floor, (500, 0)) {
            Some(sand) => { rock.insert(sand); },
            None => { break; }
        }
        num_turns += 1;
    }
    println!("{}", num_turns);
}
