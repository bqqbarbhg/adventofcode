use std::collections::HashMap;

static FACES: [(i32, i32, i32); 6] = [
    (1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1),
];

fn main() {
    let input = std::io::stdin().lines()
        .flat_map(|line| line.ok())
        .filter(|line| !line.trim().is_empty());

    let mut cells: HashMap<(i32,i32,i32),usize> = HashMap::new();
    for line in input {
        let [x, y, z]: [i32; 3] = line.split(',')
            .map(|p| p.parse::<i32>().unwrap())
            .collect::<Vec<_>>()
            .try_into()
            .unwrap();

        for (dx, dy, dz) in FACES {
            let point = (x*2+dx, y*2+dy, z*2+dz);
            *cells.entry(point).or_default() += 1;
        }
    }

    let count = cells.iter()
        .filter(|(_, &v)| v == 1)
        .count();
    println!("{}", count);
}
