extern crate derive_more;
mod common;
use common::{Map, Dir, Vector, Move, Topology, walk};
use std::cmp::max;

fn non_empty(s: &String) -> bool { !s.trim().is_empty() }

struct FlatTopology {
    boundary: [Vec<i32>; 4],
}

impl FlatTopology {
    pub fn new(map: &Map) -> FlatTopology {
        let mut topo = FlatTopology { boundary: Default::default() };

        let (w, h) = (map.width, map.height);
        for (ix, size) in [h, w, h, w].into_iter().enumerate() {
            topo.boundary[ix].resize(size as usize, i32::MIN);
        }

        for y in 0..h {
            for x in 0..w {
                let p = Vector(x, y);
                if !map.is_ground(p) { continue; }
                for dir in 0..4u8 {
                    let d = p.face(dir);
                    let bx = &mut topo.boundary[dir as usize][d.1 as usize];
                    *bx = max(*bx, d.0);
                }
            }
        }

        topo
    }
}

impl Topology for FlatTopology {
    fn wrap(&self, pos: Vector, dir: Dir) -> (Vector, Dir) {
        let p = pos.face(dir);
        if p.0 > self.boundary[dir as usize][p.1 as usize] {
            let opposite_dir = (dir + 2) % 4;
            let opposite = self.boundary[opposite_dir as usize][p.1 as usize];
            (Vector(opposite, p.1).unface(opposite_dir), dir)
        } else {
            (pos, dir)
        }
    }
}

fn main() {
    let mut input = std::io::stdin().lines()
        .flat_map(|l| l.ok());

    let map = Map::parse(input.by_ref().take_while(non_empty));
    let moves = Move::parse(&input.find(non_empty).expect("could not find moves"));
    let topo = FlatTopology::new(&map);

    let (pos, dir) = walk(&map, &topo, &moves);
    let password = (pos.1+1)*1000 + (pos.0+1)*4 + dir as i32;
    println!("{}", password);
}
