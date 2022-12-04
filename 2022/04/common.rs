#![allow(dead_code)]

pub struct Range {
    pub min: u32,
    pub max: u32,
}

impl Range {
    pub fn parse(s: &str) -> Option<Range> {
        let (min, max) = s.split_once('-')?;
        Some(Range{ min: min.parse().ok()?, max: max.parse().ok()? })
    }

    pub fn contains(&self, rhs: &Range) -> bool {
        rhs.min >= self.min && rhs.max <= self.max
    }

    pub fn overlaps(&self, rhs: &Range) -> bool {
        !(self.max < rhs.min || self.min > rhs.max)
    }
}

pub fn parse_line(line: &str) -> Option<(Range, Range)> {
    let (a, b) = line.trim().split_once(',')?;
    Some((Range::parse(a)?, Range::parse(b)?))
}
