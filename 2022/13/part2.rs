mod common;
use common::Item;

fn divider(value: u8) -> Item {
    use Item::{List, Value};
    List(vec![List(vec![Value(value)])])
}

fn main() {
    let mut input: Vec<_> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| Item::parse(l.trim()))
        .collect();

    input.push(divider(2));
    input.push(divider(6));
    input.sort();

    let a = input.iter().position(|i| *i == divider(2)).unwrap();
    let b = input.iter().position(|i| *i == divider(6)).unwrap();
    println!("{}", (a + 1) * (b + 1));
}
