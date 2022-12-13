use std::cmp::{PartialOrd, PartialEq, Ord, Eq, Ordering};

#[derive(Debug)]
pub enum Item {
    List(Vec<Item>),
    Value(u8),
}

impl Eq for Item { }
impl PartialEq for Item {
    fn eq(&self, other: &Self) -> bool {
        self.partial_cmp(other).unwrap().is_eq()
    }
}

impl PartialOrd for Item {
    fn partial_cmp(&self, other: &Self) -> Option<Ordering> {
        Some(self.cmp(other))
    }
}

impl Ord for Item {
    fn cmp(&self, other: &Self) -> Ordering {
        use Item::{List, Value};
        match (self, other) {
            (List(a), List(b)) => a.cmp(b),
            (Value(a), Value(b)) => a.cmp(b),
            (Value(a), List(b)) => vec![Value(*a)].cmp(b),
            (List(a), Value(b)) => a.cmp(&vec![Value(*b)]),
        }
    }
}

impl Item {
    pub fn parse(line: &str) -> Item {
        fn parse_imp(data: &[u8], pos: &mut usize) -> Item {
            match data[*pos] as char {
                '[' => {
                    let mut items: Vec<Item> = Vec::new();
                    *pos += 1;
                    while data[*pos] as char != ']' {
                        items.push(parse_imp(data, pos));
                        if data[*pos] as char == ',' {
                            *pos += 1;
                        }
                    }
                    *pos += 1;
                    Item::List(items)
                },
                _ => {
                    let mut num: u8 = 0;
                    while ('0'..='9').contains(&(data[*pos] as char)) {
                        num = num * 10 + (data[*pos] - '0' as u8);
                        *pos += 1;
                    }
                    Item::Value(num)
                }
            }
        }

        parse_imp(line.as_bytes(), &mut 0)
    }
}
