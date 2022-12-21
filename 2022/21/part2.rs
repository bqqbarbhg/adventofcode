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
    let humn = to_name("humn").unwrap();
    let monkeys = {
        let mut m = monkeys;
        use Expr::*;

        // root: Represent a=b as a-b=0
        m.insert(root, match m[&root] {
            Add(a, b) | Sub(a, b) | Mul(a, b) | Div(a, b) => Sub(a, b),
            _ => panic!("bad root type"),
        });

        // humn: 0 + 1*x
        m.insert(humn, Literal(Value(0.into(), 1.into())));

        m
    };

    // Evaluate the 'root' monkey
    let mut stack = vec![root];
    let mut values: HashMap<Name, Value> = HashMap::new();
    while let Some(name) = stack.pop() {
        let expr = monkeys.get(&name).unwrap();
        if let Some(value) = expr.eval(&values) {
            values.insert(name, value);
        } else {
            stack.push(name);
            stack.extend(expr.deps().as_ref());
        }
    }

    // Solve polynomial a + b*x = 0 for the result
    let Value(a, b) = values[&root];
    let result = -a / b;

    assert!(result.is_integer());
    println!("{}", result.to_integer());
}
