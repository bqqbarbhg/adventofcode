fn main() {
    let mut state: Vec<(u16, i16)> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| l.parse::<i16>().ok())
        .enumerate()
        .map(|(ix, delta)| (ix as u16, delta))
        .collect();

    let mut args = std::env::args().skip(1);
    let rounds: u64 = args.next().and_then(|s| s.parse().ok()).unwrap_or(10);
    let key: i64 = args.next().and_then(|s| s.parse().ok()).unwrap_or(811589153);

    for _ in 0..rounds {
        let num = state.len();
        for id in 0..(num as u16) {
            let src = state.iter().position(|&(ix, _)| ix == id).unwrap();
            let (_, delta) = state.remove(src);
            let key_delta = delta as i64 * key;
            let dst = (src as i64 + key_delta).rem_euclid((num - 1) as i64) as usize;
            state.insert(dst, (id, delta));
        }
    }

    let zero_pos = state.iter().position(|&(_, delta)| delta == 0).unwrap();
    let sum: i64 = [1000, 2000, 3000].into_iter()
        .map(|ix| state[(zero_pos + ix as usize) % state.len()].1 as i64)
        .map(|num| num * key)
        .sum();
    println!("{}", sum);
}
