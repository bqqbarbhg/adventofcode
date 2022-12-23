use std::collections::{HashMap, HashSet};

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

fn main() {
    let mut elves: HashSet<(i32, i32)> = std::io::stdin().lines()
        .flat_map(|line| line.ok())
        .enumerate()
        .flat_map(|(y, line)| line
            .into_bytes()
            .into_iter()
            .enumerate()
            .filter(|(_, b)| *b == '#' as u8)
            .map(move |(x, _)| (x as i32, y as i32)))
        .collect();

    let rounds: usize = std::env::args()
        .nth(1).and_then(|s| s.parse().ok()).unwrap_or(10);

    let mut proposals: HashMap<(i32, i32), ((i32, i32), usize)> = HashMap::new();
    for round in 0..rounds {
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

        for (&dst, &(src, num)) in &proposals {
            if num == 1 {
                elves.remove(&src);
                elves.insert(dst);
            }
        }
    }

    let min_x = elves.iter().map(|(x, _)| x).min().unwrap();
    let max_x = elves.iter().map(|(x, _)| x).max().unwrap();
    let min_y = elves.iter().map(|(_, y)| y).min().unwrap();
    let max_y = elves.iter().map(|(_, y)| y).max().unwrap();

    let area = (max_x - min_x + 1) * (max_y - min_y + 1) - elves.len() as i32;
    println!("{}", area);
}
