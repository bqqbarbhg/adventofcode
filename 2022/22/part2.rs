extern crate derive_more;
use derive_more::{Add, Sub, Mul, Neg};
mod common;
use common::{Map, Dir, Vector, Move, Topology, walk};
use std::cmp::{PartialEq, Eq};
use std::hash::Hash;
use std::collections::HashSet;

fn non_empty(s: &String) -> bool { !s.trim().is_empty() }

#[derive(Default, Debug, Clone, Copy, PartialEq, Eq, Hash, Add, Sub, Mul, Neg)]
struct Vector3(pub i32, pub i32, pub i32);

impl Vector3 {
    pub fn dot(self, other: Vector3) -> i32 {
        self.0*other.0 + self.1*other.1 + self.2*other.2
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Hash)]
struct Axes {
    pub x: Vector3,
    pub y: Vector3,
    pub z: Vector3,
}

impl Axes {
    pub fn adjacent(&self, dir: Dir) -> Axes {
        match dir {
            0 => Axes { x: -self.z, y: self.y, z: self.x },
            1 => Axes { x: self.x, y: -self.z, z: self.y },
            2 => Axes { x: self.z, y: self.y, z: -self.x },
            3 => Axes { x: self.x, y: self.z, z: -self.y },
            _ => unreachable!(),
        }
    }
}

#[derive(Debug, Clone, PartialEq, Eq, Hash)]
struct Face {
    pub top_left: Vector,
    pub origin: Vector3,
    pub axes: Axes,
}

impl Face {
    pub fn eval(&self, local: Vector, z: i32) -> Vector3 {
        self.origin + self.axes.x * local.0 + self.axes.y * local.1 + self.axes.z * z
    }

    pub fn uneval(&self, pos: Vector3) -> (Vector, i32) {
        let p = pos - self.origin;
        (Vector(self.axes.x.dot(p), self.axes.y.dot(p)), self.axes.z.dot(p))
    }
}

struct CubeTopology {
    faces: Vec<Face>,
    face_size: i32,
}

fn gather_faces(map: &Map, dst: &mut HashSet<Face>, parent: Face, face_size: i32) {
    if !dst.insert(parent.clone()) { return; }
    if dst.len() > 6 {
        panic!("bad topology");
    }

    for dir in 0..4u8 {
        let face = Face {
            top_left: parent.top_left + Vector(face_size, 0).turn(dir),
            origin: Default::default(),
            axes: parent.axes.adjacent(dir),
        };

        let (parent_conn, face_conn) = match dir {
            0 => (Vector(face_size - 1, 0), Vector(0, 0)),
            1 => (Vector(0, face_size - 1), Vector(0, 0)),
            2 => (Vector(0, 0), Vector(face_size - 1, 0)),
            3 => (Vector(0, 0), Vector(0, face_size - 1)),
            _ => unreachable!(),
        };
        let parent_3d = parent.eval(parent_conn, -1);
        let face_3d = face.eval(face_conn, -1);

        let face = Face { origin: parent_3d - face_3d, ..face };
        if map.is_ground(face.top_left) {
            gather_faces(map, dst, face, face_size);
        }
    }
}

fn within_face(local: Vector, face_size: i32) -> bool {
    local.0 >= 0 && local.1 >= 0 && local.0 < face_size && local.1 < face_size
}

impl CubeTopology {
    pub fn new(map: &Map, face_size: i32) -> CubeTopology {
        assert!(map.width % face_size == 0);
        assert!(map.height % face_size == 0);

        let start_x = (0..)
            .position(|mx| map.is_ground(Vector(mx * face_size, 0)))
            .expect("could not find starting face") as i32;

        let face = Face {
            top_left: Vector(start_x * face_size, 0),
            origin: Vector3(0, 0, 0),
            axes: Axes {
                x: Vector3(1, 0, 0),
                y: Vector3(0, 1, 0),
                z: Vector3(0, 0, 1),
            },
        };

        let mut faces = HashSet::new();
        gather_faces(map, &mut faces, face, face_size);
        if faces.len() != 6 {
            panic!("bad topology");
        }

        CubeTopology {
            faces: faces.into_iter().collect(),
            face_size: face_size,
        }
    }
}

impl Topology for CubeTopology {
    fn wrap(&self, pos: Vector, dir: Dir) -> (Vector, Dir) {
        let prev_pos = pos + Vector(-1, 0).turn(dir);
        let face = self.faces.iter()
            .find(|f| within_face(prev_pos - f.top_left, self.face_size))
            .expect("previous position not in a face");

        if within_face(pos - face.top_left, self.face_size) {
            return (pos, dir);
        }

        let next_z = face.axes.adjacent(dir).z;
        let next_face = self.faces.iter()
            .find(|f| f.axes.z == next_z)
            .expect("could not find next face");

        let pos_3d = face.eval(prev_pos - face.top_left, -1);
        let dir_3d = face.eval(prev_pos - face.top_left, -2);
        let (next_local, next_check) = next_face.uneval(pos_3d);
        let (dir_local, dir_check) = next_face.uneval(dir_3d);
        assert_eq!(next_check, -1);
        assert_eq!(dir_check, -1);

        let delta = dir_local - next_local;
        let next_dir: u8 = match delta {
            Vector(1, 0) => 0,
            Vector(0, 1) => 1,
            Vector(-1, 0) => 2,
            Vector(0, -1) => 3,
            _ => panic!("bad step"),
        };

        (next_face.top_left + next_local, next_dir)
    }
}

fn main() {
    let mut input = std::io::stdin().lines()
        .flat_map(|l| l.ok());

    let face_size: i32 = std::env::args()
        .nth(1).and_then(|a| a.parse().ok()).unwrap_or(50);

    let map = Map::parse(input.by_ref().take_while(non_empty));
    let moves = Move::parse(&input.find(non_empty).expect("could not find moves"));
    let topo = CubeTopology::new(&map, face_size);

    let (pos, dir) = walk(&map, &topo, &moves);
    let password = (pos.1+1)*1000 + (pos.0+1)*4 + dir as i32;
    println!("{}", password);
}
