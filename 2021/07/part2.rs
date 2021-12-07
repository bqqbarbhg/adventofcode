use std::io::{self, Read};
use std::cmp::{min, max};

fn main() {
    // Split input into numbers representing crab positions and sort them.
    let mut input = String::new();
    io::stdin().read_to_string(&mut input).unwrap();
    let mut positions: Vec<i32> = input
        .split(",")
        .flat_map(|line| line.parse().ok())
        .collect();
    let num = positions.len() as i32;
    positions.sort();

    // part1.rs `scan()` update step:
    //   |split, (ix, &pos)| {
    //       let delta = pos - split.pos;
    //       let num_left = ix as i32;
    //       let num_right = num - num_left;
    //       split.cost = split.cost + (num_left - num_right) * delta;
    //       split.pos = pos;
    //       Some(split.clone())
    //   }
    // Analyzing a single step from part1.rs we can see that whether `cost`
    // grows or shrinks is only based on `num_left - num_right` as `delta >= 0`.
    // As `num_left` goes from `0 -> num` and `num_right` `num -> 0`, `cost`
    // first decreases until `num_left >= num_right` and afterwards increases.
    // Thus the minimum position is at the middle of the array ie. the median.
    let median = positions[(num / 2) as usize];

    // Our new metric for a step of `n` is `sum(0..n)` which is in closed form
    // a triangle number: `n*(n+1)/2 == (n^2 + n)/2`.
    //     sum(0..n)
    //   = n*(n+1) / 2
    //   = (n^2 + n) / 2
    // We can drop the constant factor from the cost function and only minimize
    // the expression `n^2 + n` for position `a`:
    //     sum( |x-a|^2 + |x-a| )
    //   = sum(|x-a|^2) + sum(|x-a|)
    //   = sum((x-a)^2) + sum(|x-a|)
    // Let's denote
    //     d1(a) = sum(|x-a|)
    //     d2(a) = sum((x-a)^2)
    // We have computed the minimum position `median` of `d1` above so let's
    // consider potential points that are smaller than that:
    //     d1(a) + d2(a) <= d1(median) + d2(median)
    //     d1(a)-d1(median) + d2(a) <= d2(median)
    // As `d1(median)` is the minimum `d1(a) >= d1(median)` for all `a` so we
    // can drop that expression and solve for a looser inequality:
    //     d2(a) <= d2(median)
    // Since `d2(a)` is the square error from `a` we know it's minimum is located
    // at `mean = sum(x)/len(x)`. Both functions are convex we know that the
    // minimum of the sum `d1(a) + d2(b)` must be between their respective minimum
    // points `median` and `mean`!
    let total: f64 = positions.iter().map(|&x| x as f64).sum();
    let mean = (total / num as f64).round() as i32;

    // Fudge the bounds a bit to account for rounding and median of even sizes.
    let begin = min(mean, median) - 1;
    let end = max(mean, median) + 1;

    // Brute force the rest
    let best_cost: i32 = (begin ..= end)
        .map(|pos| {
            positions.iter()
                .map(|&x| {
                    let n = (pos - x).abs();
                    (n*(n+1))/2
                })
                .sum()
        })
        .min()
        .unwrap();

    println!("{}", best_cost);
}
