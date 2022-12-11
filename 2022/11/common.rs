use pest::{Parser, error::Error, iterators::Pair};

#[derive(Debug, Clone, Copy)]
pub enum Argument {
    Literal(i64),
    Input,
}

impl Argument {
    pub fn eval(&self, input: i64) -> i64 {
        match self {
            Argument::Literal(v) => *v,
            Argument::Input => input,
        }
    }
}

#[derive(Debug)]
pub enum Operation {
    Add(Argument, Argument),
    Mul(Argument, Argument),
}

impl Operation {
    pub fn eval(&self, input: i64) -> i64 {
        match self {
            Operation::Add(a, b) => a.eval(input) + b.eval(input),
            Operation::Mul(a, b) => a.eval(input) * b.eval(input),
        }
    }
}

#[derive(Debug)]
pub struct Monkey {
    pub index: i64,
    pub items: Vec<i64>,
    pub operation: Operation,
    pub test: i64,
    pub if_true: i64,
    pub if_false: i64,
    pub activity: usize,
}

#[derive(Parser)]
#[grammar = "11/monkey.pest"]
struct MonkeyParser;

pub fn parse_monkeys(source: &str) -> Result<Vec<Monkey>, Error<Rule>> {
    let file = MonkeyParser::parse(Rule::file, source)?.next().unwrap();

    fn first(pair: Pair<Rule>) -> Pair<Rule> { pair.into_inner().next().unwrap() }
    fn take2(pair: Pair<Rule>) -> (Pair<Rule>, Pair<Rule>) {
        let mut it = pair.into_inner();
        (it.next().unwrap(), it.next().unwrap())
    }
    fn take3(pair: Pair<Rule>) -> (Pair<Rule>, Pair<Rule>, Pair<Rule>) {
        let mut it = pair.into_inner();
        (it.next().unwrap(), it.next().unwrap(), it.next().unwrap())
    }
    fn number(pair: Pair<Rule>) -> i64 { pair.as_str().parse().unwrap() }

    fn items(pair: Pair<Rule>) -> Vec<i64> {
        pair.into_inner().map(|v| v.as_str().parse().unwrap()).collect()
    }

    fn argument(pair: Pair<Rule>) -> Argument {
        let pair = first(pair);
        match pair.as_rule() {
        Rule::number => Argument::Literal(number(pair)),
        Rule::old => Argument::Input,
        _ => unreachable!(),
        }
    }

    fn operation(pair: Pair<Rule>) -> Operation {
        let (a, op, b) = take3(pair);
        let a = argument(a);
        let b = argument(b);
        match op.as_str() {
            "+" => Operation::Add(a, b),
            "*" => Operation::Mul(a, b),
            _ => unreachable!(),
        }
    }

    fn parse_monkey(pair: Pair<Rule>) -> Monkey {
        assert!(pair.as_rule() == Rule::monkey);
        let mut it = pair.into_inner();
        let mut monkey = Monkey {
            index: number(first(it.next().unwrap())),
            items: items(it.next().unwrap()),
            operation: operation(it.next().unwrap()),
            test: number(first(it.next().unwrap())),
            if_false: -1,
            if_true: -1,
            activity: 0,
        };

        for pair in it {
            let (b, ix) = take2(pair);
            let ix = number(ix);
            match b.as_str() {
                "true" => { monkey.if_true = ix },
                "false" => { monkey.if_false = ix },
                _ => unreachable!(),
            };
        }
        
        monkey
    }

    Ok(file.into_inner().map(parse_monkey).collect())
}
