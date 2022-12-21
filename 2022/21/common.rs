use std::collections::HashMap;
use std::convert::AsRef;
use std::ops::{Add, Sub, Mul, Div};
use num_rational::Rational64;

pub type Name = [u8; 4];
pub fn to_name(s: &str) -> Option<Name> {
    s.as_bytes().try_into().ok()
}

pub struct Deps([Name; 2], usize);
impl AsRef<[Name]> for Deps {
    fn as_ref(&self) -> &[Name] {
        &self.0[..self.1]
    }
}

// (a/b) + (c/d)*x
#[derive(Debug, Clone, Copy)]
pub struct Value(pub Rational64, pub Rational64);

impl Add for Value {
    type Output = Self;
    fn add(self, rhs: Self) -> Self::Output {
        Value(self.0 + rhs.0, self.1 + rhs.1)
    }
}

impl Sub for Value {
    type Output = Self;
    fn sub(self, rhs: Self) -> Self::Output {
        Value(self.0 - rhs.0, self.1 - rhs.1)
    }
}

impl Mul for Value {
    type Output = Self;
    fn mul(self, rhs: Self) -> Self::Output {
        assert!(self.1 == 0.into() || rhs.1 == 0.into());
        Value(self.0 * rhs.0, self.0 * rhs.1 + self.1 * rhs.0)
    }
}

impl Div for Value {
    type Output = Self;
    fn div(self, rhs: Self) -> Self::Output {
        assert!(rhs.1 == 0.into());
        Value(self.0 / rhs.0, self.1 / rhs.0)
    }
}

pub enum Expr {
    Literal(Value),
    Add(Name, Name),
    Sub(Name, Name),
    Mul(Name, Name),
    Div(Name, Name),
}

impl Expr {
    pub fn parse(s: &str) -> Option<Expr> {
        let s = s.trim();
        if let Ok(val) = s.parse::<i64>() {
            return Some(Expr::Literal(Value(val.into(), 0.into())));
        }

        let (a, rest) = s.split_once(' ')?;
        let (op, b) = rest.split_once(' ')?;
        let (a, b) = (to_name(a)?, to_name(b)?);
        match op {
            "+" => Some(Expr::Add(a, b)),
            "-" => Some(Expr::Sub(a, b)),
            "*" => Some(Expr::Mul(a, b)),
            "/" => Some(Expr::Div(a, b)),
            _ => None,
        }
    }

    pub fn eval(&self, values: &HashMap<Name, Value>) -> Option<Value> {
        use Expr::*;
        let arg = |name| values.get(name).map(|v| *v);
        let result = match self {
            Literal(v) => *v,
            Add(a, b) => arg(a)? + arg(b)?,
            Sub(a, b) => arg(a)? - arg(b)?,
            Mul(a, b) => arg(a)? * arg(b)?,
            Div(a, b) => arg(a)? / arg(b)?,
        };
        Some(result)
    }

    pub fn deps(&self) -> Deps {
        use Expr::*;
        match self {
            Literal(_) => Deps([[0; 4]; 2], 0),
            Add(a, b) | Sub(a, b) | Mul(a, b) | Div(a, b) => Deps([*a, *b], 2),
        }
    }
}
