extern crate derive_more;
mod common;
use common::{Vector, StateInfo, StateNext, Scene, parse_scene, search};

static DIRS: [Vector; 4] = [
    Vector(-1, 0), Vector(1, 0), Vector(0, -1), Vector(0, 1),
];

fn state_info(scene: &Scene, state: Vector) -> StateInfo<Vector> {
    if state == scene.end { return StateInfo::Goal; }

    let base_height = scene.grid.get(state).unwrap();
    let end_height = 'z' as i32 - 'a' as i32;

    let next = DIRS.iter()
        .flat_map(|&dir| -> Option<StateNext<Vector>> {
            let pos = Vector(state.0 + dir.0, state.1 + dir.1);
            let height = scene.grid.get(pos)?;

            let rise = (height as u64).saturating_sub(base_height as u64);
            let end_rise = (end_height as u64).saturating_sub(height as u64);
            if rise > 1 { return None }

            let (dx, dy) = (scene.end.0 - pos.0, scene.end.1 - pos.1);
            Some(StateNext {
                state: pos,
                cost: 1,
                heuristic: end_rise.max((dx.abs() + dy.abs()) as u64),
            })
        })
        .collect();
    StateInfo::Node(next)
}

fn main() {
    let input = std::io::stdin().lines().flat_map(|l| l.ok());
    let scene = parse_scene(input);

    let path = search(scene.start, |s| state_info(&scene, s))
        .expect("could not find path");

    println!("{}", path.len() - 1)
}
