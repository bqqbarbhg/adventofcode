mod common;
use common::{Op, Vm};

fn main() {
    let code: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| Op::parse(&l))
        .collect();
    
    let mut vm = Vm::new(code);

    let image: String = (0..240)
        .map(|cycle| {
            vm.step();
            (vm.x - cycle % 40).abs() <= 1
        })
        .map(|v| if v { '#' } else { ' ' })
        .collect();

    for line in image.as_bytes().chunks(40) {
        println!("{}", std::str::from_utf8(line).unwrap());
    }
}
