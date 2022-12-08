mod common;
use common::{Grid, Point, Direction, parse_grid};
use std::collections::HashSet;

fn collect_visible(visible: &mut HashSet<Point>, grid: &Grid<u8>, dir: Direction) {
    let size = dir.map_size(grid.size());
    let (width, height) = size;
    let mut highest = vec![0u8; width];

    for y in 0..height {
        for x in 0..width {
            let point = dir.map_point(size, (x, y));
            let height = grid[point];
            if height > highest[x] {
                visible.insert(point);
                highest[x] = height;
            }
        }
    }
}

fn main() {
    let input = std::io::stdin().lines().flat_map(|l| l.ok());
    let grid = parse_grid(input);

    let mut visible = HashSet::<Point>::new();
    collect_visible(&mut visible, &grid, Direction::Top);
    collect_visible(&mut visible, &grid, Direction::Bottom);
    collect_visible(&mut visible, &grid, Direction::Left);
    collect_visible(&mut visible, &grid, Direction::Right);

    println!("{}", visible.len());
}
