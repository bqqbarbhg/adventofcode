mod common;
use common::{StateInfo, StateNext, search};

fn make_bits(w: usize, h: usize, mut f: impl FnMut(usize, usize) -> bool) -> Vec<u128> {
    (0..h).map(|y| (0..w)
        .map(|x| (f(x as usize, y as usize) as u128) << x)
        .fold(0u128, |a, b| a | b))
        .collect()
}

fn has_bit(mask: u128, bit: i32) -> bool {
    (mask >> bit & 1) != 0
}

static DIRS: [(i32, i32); 5] = [
    (0, 0), (-1, 0), (1, 0), (0, -1), (0, 1),
];

fn main() {
    let input: Vec<Vec<char>> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .map(|l| l.trim().chars().collect())
        .collect();

    let width = input[0].len();
    let height = input.len();
    let wall = make_bits(width, height, |x, y| input[y][x] == '#');
    let left = make_bits(width, height, |x, y| input[y][x] == '<');
    let right = make_bits(width, height, |x, y| input[y][x] == '>');
    let up = make_bits(height, width, |x, y| input[x][y] == '^');
    let down = make_bits(height, width, |x, y| input[x][y] == 'v');

    type State = (i32, i32, i32);
    let free = |(x, y, t): State| {
        let (w, h) = (width as i32, height as i32);
        if x < 0 || y < 0 || x >= w || y >= h { return false; }
        let mut r = has_bit(wall[y as usize], x as i32);
        r |= has_bit(left[y as usize], 1 + (x - 1 + t).rem_euclid(w - 2));
        r |= has_bit(right[y as usize], 1 + (x - 1 - t).rem_euclid(w - 2));
        r |= has_bit(up[x as usize], 1 + (y - 1 + t).rem_euclid(h - 2));
        r |= has_bit(down[x as usize], 1 + (y - 1 - t).rem_euclid(h - 2));
        !r
    };

    let top = (wall[0].trailing_ones() as i32, 0);
    let bottom = (wall[height as usize - 1].trailing_ones() as i32, height as i32 - 1);

    let mut start = (top.0, top.1, 0);
    for goal in [bottom, top, bottom] {
        let next = |(x, y, t): State| -> StateInfo<State> {
            if y == goal.1 { return StateInfo::Goal; }
            let next: Vec<_> = DIRS.iter()
                .map(|(dx, dy)| (x + dx, y + dy, t + 1))
                .flat_map(|s| free(s).then_some(s))
                .map(|(x, y, t)| StateNext {
                    state: (x, y, t),
                    cost: 1,
                    heuristic: (x.abs_diff(goal.0) + y.abs_diff(goal.1)) as u64,
                })
                .collect();
            StateInfo::Node(next)
        };
        let path = search(start, next)
            .expect("could not find path");
        start = *path.last().unwrap();
    }

    println!("{}", start.2);
}
