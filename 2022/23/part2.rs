use fxhash::{FxHashMap, FxHashSet};

type Pattern = ([(i32, i32); 3], (i32, i32));
static PATTERNS: [Pattern; 4] = [
    ([(-1,-1), (0,-1), (1,-1)], (0,-1)),
    ([(-1,1), (0,1), (1,1)], (0,1)),
    ([(-1,-1), (-1,0), (-1,1)], (-1,0)),
    ([(1,-1), (1,0), (1,1)], (1,0)),
];

static ADJACENT: [(i32, i32); 8] = [
    (-1,-1), (0,-1), (1,-1), (-1,0), (1,0), (-1,1), (0,1), (1,1),
];

fn patterns(offset: usize) -> impl Iterator<Item=&'static Pattern> {
    PATTERNS.iter()
        .cycle()
        .skip(offset % 4)
        .take(4)
}

fn simulate(mut elves: FxHashSet<(i32, i32)>) -> usize {
    let mut proposals: FxHashMap<(i32, i32), ((i32, i32), usize)> = FxHashMap::default();
    for round in 0.. {
        proposals.clear();
        for &(x, y) in &elves {
            if !ADJACENT.iter().any(|(ax, ay)| elves.contains(&(x+ax, y+ay))) {
                continue;
            }
            for (check, (dx, dy)) in patterns(round) {
                if check.iter().all(|(cx, cy)| !elves.contains(&(x+cx, y+cy))) {
                    proposals.entry((x+dx, y+dy)).or_insert(((x, y), 0)).1 += 1;
                    break;
                }
            }
        }

        let mut moved = false;
        for (&dst, &(src, num)) in &proposals {
            if num == 1 {
                elves.remove(&src);
                elves.insert(dst);
                moved = true;
            }
        }
        if !moved {
            return round + 1;
        }
    }
    unreachable!();
}

fn main() {
    let elves: FxHashSet<(i32, i32)> = std::io::stdin().lines()
        .flat_map(|line| line.ok())
        .enumerate()
        .flat_map(|(y, line)| line
            .into_bytes()
            .into_iter()
            .enumerate()
            .filter(|(_, b)| *b == '#' as u8)
            .map(move |(x, _)| (x as i32, y as i32)))
        .collect();

    let rounds = simulate(elves);
    println!("{}", rounds);
}

