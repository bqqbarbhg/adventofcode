mod common;
use common::Item;

fn main() {
    let mut input = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| Item::parse(l.trim()));

    let mut ordered = 0;
    for ix in 1.. {
        let a = input.next();
        let b = input.next();
        if let (Some(a), Some(b)) = (a, b) {
            if a <= b { ordered += ix; }
        } else {
            break;
        }
    }
    println!("{}", ordered);
}
