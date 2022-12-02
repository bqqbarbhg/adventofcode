use std::slice;

pub enum Shape {
    Rock,
    Paper,
    Scissors,
}

impl Shape {
    pub fn parse(c: &str) -> Option<Shape> {
        match c {
        "A"|"X" => Some(Shape::Rock),
        "B"|"Y" => Some(Shape::Paper),
        "C"|"Z" => Some(Shape::Scissors),
        _ => None,
        }
    }

    pub fn score(&self) -> i32 {
        match self {
            Shape::Rock => 1,
            Shape::Paper => 2,
            Shape::Scissors => 3,
        }
    }

    pub fn beats(&self, other: &Shape) -> bool {
        match (self, other) {
        (Shape::Rock, Shape::Scissors) => true,
        (Shape::Paper, Shape::Rock) => true,
        (Shape::Scissors, Shape::Paper) => true,
        _ => false,
        }
    }

    pub fn iter() -> slice::Iter<'static, Shape> {
        static SHAPES: [Shape; 3] = [Shape::Rock, Shape::Paper, Shape::Scissors];
        SHAPES.iter()
    }
}
