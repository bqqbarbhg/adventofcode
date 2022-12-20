fn main() {
    let mut state: Vec<(u16, i16)> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| l.parse::<i16>().ok())
        .enumerate()
        .map(|(ix, delta)| (ix as u16, delta))
        .collect();

    let num = state.len();
    for id in 0..(num as u16) {
        let src = state.iter().position(|&(ix, _)| ix == id).unwrap();
        let (_, delta) = state.remove(src);
        let dst = (src as i16 + delta).rem_euclid((num - 1) as i16) as usize;
        state.insert(dst, (id, delta));
    }
    
    let zero_pos = state.iter().position(|&(_, delta)| delta == 0).unwrap();
    let sum: i32 = [1000, 2000, 3000].into_iter()
        .map(|ix| state[(zero_pos + ix as usize) % state.len()].1 as i32)
        .sum();
    println!("{}", sum);
}
