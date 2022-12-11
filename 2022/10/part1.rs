mod common;
use common::{Op, Vm};

fn main() {
    let code: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| Op::parse(&l))
        .collect();
    
    let mut vm = Vm::new(code);

    let mut total_strength = 0;
    for _ in 0..=220 {
        vm.step();
        if (vm.cycle + 20) % 40 == 0 {
            total_strength += vm.x * vm.cycle as i64;
        }
    }
    println!("{}", total_strength);
}
