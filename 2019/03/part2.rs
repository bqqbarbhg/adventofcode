use std::io::prelude::*;

#[derive(Copy, Clone, Debug)]
struct Point {
    x: i32, y: i32,
}

#[derive(Debug)]
struct Span {
    pos: i32, // Position perpendicular to the span axis
    min: i32, max: i32, // Inclusive bounds along the span axis
    t_bias: i32, t_scale: i32, // Mapping from position along axis to wire time
}

#[derive(Debug)]
enum Line {
    X(Span), // Horizontal
    Y(Span), // Vertical
}

#[derive(Default)]
struct Spans {
    x: Vec<Span>,
    y: Vec<Span>,
}

fn parse_line(token: &str, pos: &mut Point, time: &mut i32) -> Line {
    let begin = pos.clone();
    let (dir, num) = token.split_at(1);
    let num: i32 = num.parse().unwrap();
    let t = *time;
    *time = t + num;
    match dir {
        "L" => {
            pos.x -= num;
            Line::X(Span { pos: pos.y, min: pos.x, max: begin.x, t_bias: t + begin.x, t_scale: -1 })
        },
        "R" => {
            pos.x += num;
            Line::X(Span { pos: pos.y, min: begin.x, max: pos.x, t_bias: t - begin.x, t_scale: 1 })
        },
        "D" => {
            pos.y -= num;
            Line::Y(Span { pos: pos.x, min: pos.y, max: begin.y, t_bias: t + begin.y, t_scale: -1 })
        },
        "U" => {
            pos.y += num;
            Line::Y(Span { pos: pos.x, min: begin.y, max: pos.y, t_bias: t - begin.y, t_scale: 1 })
        },
        _ => panic!("Unexpected direction {}", dir)
    }
}

fn intersect(spans: &[Span], span: &Span) -> Vec<(Point, i32, i32)> {
    // Binary search first span where `pos >= min`
    let mut ix = {
        let mut lo: usize = 0;
        let mut hi: usize = spans.len() - 1;
        while lo < hi {
            let mid = (lo + hi) / 2;
            if spans[mid].pos < span.min {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        lo
    };

    // Intersect spans until `pos <= max`
    let mut res = Vec::<(Point, i32, i32)>::new();
    while ix < spans.len() && spans[ix].pos <= span.max {
        let other = &spans[ix];
        if span.pos >= other.min && span.pos <= other.max {
            let ta = span.t_bias + span.t_scale * other.pos;
            let tb = other.t_bias + other.t_scale * span.pos;
            res.push((Point { x: span.pos, y: other.pos }, ta, tb));
        }
        ix += 1;
    }
    res
}

fn main() {
    let stdin = std::io::stdin();
    let mut input = stdin.lock().lines();

    let mut spans: Spans = Default::default();

    // Collect spans for A
    {
        let a = input.next().unwrap().unwrap();
        let mut pos = Point { x: 0, y: 0 };
        let mut time = 0;
        for token in a.split(',') {
            match parse_line(token, &mut pos, &mut time) {
                Line::X(span) => spans.x.push(span),
                Line::Y(span) => spans.y.push(span),
            }
        }

        spans.x.sort_unstable_by_key(|s| s.pos);
        spans.y.sort_unstable_by_key(|s| s.pos);
    }

    let mut closest = std::i32::MAX;

    // Intersect spans for B
    {
        let b = input.next().unwrap().unwrap();
        let mut pos = Point { x: 0, y: 0 };
        let mut time = 0;
        for token in b.split(',') {
            let line = parse_line(token, &mut pos, &mut time);
            let isect = match &line {
                Line::X(span) => intersect(&spans.y, &span),
                Line::Y(span) => intersect(&spans.x, &span),
            };
            for (_, ta, tb) in isect {
                let t = ta + tb;
                if t > 0 && t < closest {
                    closest = t;
                }
            }
        }
    }

    println!("{}", closest)
}


