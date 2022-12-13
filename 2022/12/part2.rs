extern crate derive_more;
mod common;
use common::{Vector, StateInfo, StateNext, Scene, parse_scene, search};

static DIRS: [Vector; 4] = [
    Vector(-1, 0), Vector(1, 0), Vector(0, -1), Vector(0, 1),
];

fn state_info(scene: &Scene, state: Vector) -> StateInfo<Vector> {
    let base_height = scene.grid.get(state).unwrap();
    if base_height == 0 { return StateInfo::Goal; }

    let next = DIRS.iter()
        .flat_map(|&dir| -> Option<StateNext<Vector>> {
            let pos = Vector(state.0 + dir.0, state.1 + dir.1);
            let height = scene.grid.get(pos)?;

            let fall = (base_height as u64).saturating_sub(height as u64);
            if fall > 1 { return None }
            Some(StateNext {
                state: pos,
                cost: 1,
                heuristic: height as u64,
            })
        })
        .collect();
    StateInfo::Node(next)
}

fn main() {
    let input = std::io::stdin().lines().flat_map(|l| l.ok());
    let scene = parse_scene(input);

    let path = search(scene.end, |s| state_info(&scene, s))
        .expect("could not find path");

    println!("{}", path.len() - 1)
}
