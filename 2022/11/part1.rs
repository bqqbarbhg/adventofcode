use std::io::Read;
use std::mem;

extern crate pest;
#[macro_use]
extern crate pest_derive;

mod common;
fn main() {
    let mut input = String::new();
    std::io::stdin().read_to_string(&mut input).unwrap();
    let mut monkeys = common::parse_monkeys(&input).unwrap();
    for (ix, m) in monkeys.iter().enumerate() { assert!(m.index == ix as i64) }

    for _ in 0..20 {
        for ix in 0..monkeys.len() {
            let mut sends: Vec<(i64, i64)> = Vec::new();

            let monkey = &mut monkeys[ix];
            monkey.activity += monkey.items.len();
            for item in mem::take(&mut monkey.items) {
                let item = monkey.operation.eval(item) / 3;
                let recipient = match item % monkey.test == 0 {
                    true => monkey.if_true,
                    false => monkey.if_false,
                };
                sends.push((recipient, item))
            }

            for (recipient, item) in sends {
                monkeys[recipient as usize].items.push(item);
            }
        }
    }

    let mut top = monkeys.iter().map(|m| m.activity).collect::<Vec<_>>();
    top.sort_by_key(|t| std::usize::MAX - *t);
    println!("{}", top[0] * top[1]);
}
