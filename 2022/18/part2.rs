use std::collections::{HashMap, HashSet};

static FACES: [(i32, i32, i32); 6] = [
    (1,0,0), (-1,0,0), (0,1,0), (0,-1,0), (0,0,1), (0,0,-1),
];

fn main() {
    let input = std::io::stdin().lines()
        .flat_map(|line| line.ok())
        .filter(|line| !line.trim().is_empty());

    let points: HashSet<_> = input
        .map(|line| {
            let [x, y, z]: [i32; 3] = line.split(',')
                .map(|p| p.parse::<i32>().unwrap())
                .collect::<Vec<_>>()
                .try_into()
                .unwrap();
            (x, y, z)
        })
        .collect();

    let min = points.iter()
        .map(|&(x, y, z)| (x-1, y-1, z-1))
        .reduce(|(x0,y0,z0), (x1,y1,z1)| (x0.min(x1), y0.min(y1), z0.min(z1)))
        .unwrap();
    let max = points.iter()
        .map(|&(x, y, z)| (x+1, y+1, z+1))
        .reduce(|(x0,y0,z0), (x1,y1,z1)| (x0.max(x1), y0.max(y1), z0.max(z1)))
        .unwrap();

    let outside = {
        let mut result: HashSet<(i32, i32, i32)> = HashSet::new();
        let mut work: Vec<(i32, i32, i32)> = vec![min];
        while let Some((x, y, z)) = work.pop() {
            for (dx, dy, dz) in FACES {
                let p = (x + dx, y + dy, z + dz);
                if result.contains(&p) || points.contains(&p) { continue; }
                if p.0 < min.0 || p.1 < min.1 || p.2 < min.2
                    || p.0 > max.0 || p.1 > max.1 || p.2 > max.2 { continue; }

                work.push(p);
                result.insert(p);
            }
        }
        result
    };

    let mut cells: HashMap<(i32,i32,i32),usize> = HashMap::new();
    for (x, y, z) in points {
        for (dx, dy, dz) in FACES {
            if !outside.contains(&(x+dx, y+dy, z+dz)) { continue; }
            let point = (x*2+dx, y*2+dy, z*2+dz);
            *cells.entry(point).or_default() += 1;
        }
    }

    let count = cells.iter()
        .filter(|(_, &v)| v == 1)
        .count();

    println!("{}", count);
}
