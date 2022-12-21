mod common;
use common::{to_name, Name, Value, Expr};
use std::collections::HashMap;

fn main() {
    let monkeys: HashMap<_, _> = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .filter(|l| !l.trim().is_empty())
        .map(|l| {
            let (name, expr) = l.split_once(":").unwrap();
            (to_name(name).unwrap(), Expr::parse(expr).unwrap())
        })
        .collect();

    let root = to_name("root").unwrap();

    // Evaluate the 'root' monkey
    let mut values: HashMap<Name, Value> = HashMap::new();
    let mut stack = vec![root];
    while let Some(name) = stack.pop() {
        let expr = monkeys.get(&name).unwrap();
        if let Some(value) = expr.eval(&values) {
            values.insert(name, value);
        } else {
            stack.push(name);
            stack.extend(expr.deps().as_ref());
        }
    }

    println!("{}", values[&root].0);
}
