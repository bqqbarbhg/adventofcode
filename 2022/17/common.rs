use std::cmp::max;

pub static SHAPES: [&'static str; 5] = [
"
####
",

"
.#.
###
.#.
",

"
..#
..#
###
",

"
#
#
#
#
",

"
##
##
",
];

#[derive(Debug, Clone, Copy)]
pub struct Shape {
    pub width: u8,
    pub height: u8,
    pub mask: u16,
}

impl Shape {
    pub fn parse(s: &str) -> Shape {
        let mut width = 0;
        let mut height = 0;
        let mut mask: u16 = 0;

        let lines: Vec<_> = s.split("\n")
            .map(|s| s.trim())
            .filter(|s| !s.is_empty())
            .collect();

        for (ix, line) in lines.iter().rev().enumerate() {
            width = max(width, line.len());
            height = ix + 1;

            let bits: u16 = line.chars()
                .enumerate()
                .map(|(ix, c)| if c == '#' { 1u16 << ix } else { 0 })
                .sum();
            mask |= bits << (ix * 4);
        }

        Shape{
            width: width as u8,
            height: height as u8,
            mask: mask,
        }
    }

    pub fn line(&self, x: u8, y: u8) -> u8 {
        assert!(y < self.height);
        (((self.mask >> (y * 4)) & 0xf) as u8) << x
    }

    pub fn intersect(&self, masks: &[u8], x: u8) -> bool {
        for y in 0..self.height {
            if (masks[y as usize] & self.line(x, y)) != 0 {
                return true;
            }
        }
        false
    }

    pub fn blit(&self, mask: &mut [u8], x: u8) {
        for y in 0..self.height {
            mask[y as usize] |= self.line(x, y);
        }
    }
}
