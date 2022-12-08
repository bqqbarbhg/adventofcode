#![allow(dead_code)]
use std::ops;

pub struct Grid<T> {
    width: usize,
    height: usize,
    values: Vec<T>,
}

pub type Point = (usize, usize);

impl<T> Grid<T> {
    pub fn size(&self) -> Point { (self.width, self.height) }
}

impl<T> Grid<T> where T: Clone {
    pub fn new((width, height): Point, value: T) -> Grid<T> {
        Grid { width: width, height: height, values: vec![value.clone(); width*height] }
    }
}

impl<T> ops::Index<Point> for Grid<T> {
    type Output = T;
    fn index(&self, (x, y): Point) -> &Self::Output {
        assert!(x < self.width && y < self.height);
        &self.values[y * self.width + x]
    }
}

impl<T> ops::IndexMut<Point> for Grid<T> {
    fn index_mut(&mut self, (x, y): Point) -> &mut Self::Output {
        assert!(x < self.width && y < self.height);
        &mut self.values[y * self.width + x]
    }
}

#[derive(Clone, Copy)]
pub enum Direction {
    Top,
    Bottom,
    Left,
    Right,
}

impl Direction {
    pub fn map_size(&self, (w, h): Point) -> Point {
        match self {
            Direction::Top | Direction::Bottom => (w, h),
            Direction::Left | Direction::Right => (h, w),
        }
    }

    pub fn map_point(&self, (_w, h): Point, (x, y): Point) -> Point {
        match self {
            Direction::Top => (x, y),
            Direction::Bottom => (x, h - y - 1),
            Direction::Left => (y, x),
            Direction::Right => (h - y - 1, x),
        }
    }
}

pub fn parse_grid(lines: impl Iterator<Item=String>) -> Grid<u8> {
    let rows: Vec<Vec<u8>> = lines
        .map(|l| l.chars()
            .flat_map(|c| c.to_digit(10))
            .map(|c| (c + 1) as u8)
            .collect::<Vec<u8>>())
        .filter(|l| !l.is_empty())
        .collect();

    let width = rows.iter().map(|r| r.len()).max()
        .expect("Expected at least one non-empty row");
    let height = rows.len();
    let mut grid = Grid::new((width, height), 0);

    for (y, row) in rows.into_iter().enumerate() {
        for (x, value) in row.into_iter().enumerate() {
            grid[(x, y)] = value;
        }
    }

    grid
}
