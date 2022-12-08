mod common;
use common::{Grid, Direction, parse_grid};

fn scenic_score(grid: &Grid<u8>, dir: Direction) -> Grid<usize> {
    let mut score = Grid::new(grid.size(), 0);
    let size = dir.map_size(grid.size());
    let (width, height) = size;
    let mut blocker_y = vec![vec![0usize; width]; 11];

    for y in 0..height {
        for x in 0..width {
            let point = dir.map_point(size, (x, y));
            let height = grid[point] as usize;
            score[point] = y - blocker_y[height][x];
            for h in 0..=height {
                blocker_y[h][x] = y;
            }
        }
    }

    score
}

fn main() {
    let input = std::io::stdin().lines().flat_map(|l| l.ok());
    let grid = parse_grid(input);

    let t = scenic_score(&grid, Direction::Top);
    let b = scenic_score(&grid, Direction::Bottom);
    let l = scenic_score(&grid, Direction::Left);
    let r = scenic_score(&grid, Direction::Right);

    let (width, height) = grid.size();

    let mut best_score = 0;
    for y in 0..height {
        for x in 0..width {
            let p = (x, y);
            best_score = best_score.max(t[p] * b[p] * l[p] * r[p]);
        }
    }
    println!("{}", best_score);
}
