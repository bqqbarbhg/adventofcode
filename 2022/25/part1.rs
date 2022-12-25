
fn parse(s: &str) -> Option<i64> {
    let mut result = 0;

    for c in s.chars() {
        let digit = match c {
            '=' => -2,
            '-' => -1,
            '0' => 0,
            '1' => 1,
            '2' => 2,
            _ => return None,
        };
        result = result * 5 + digit;
    }

    Some(result)
}

fn format(mut v: i64) -> String {
    assert!(v >= 0);
    if v == 0 { return String::from("0"); }

    let mut result: Vec<u8> = Vec::new();
    while v != 0 {
        let (offset, digit) = match v % 5 {
            0 => (0, '0'),
            1 => (1, '1'),
            2 => (2, '2'),
            3 => (-2, '='),
            4 => (-1, '-'),
            _ => unreachable!(),
        };
        result.push(digit as u8);
        v -= offset;
        v /= 5;
    }

    result.reverse();
    String::from_utf8(result).unwrap()
}

fn main() {
    let result: i64 = std::io::stdin().lines()
        .flat_map(|l| l.ok())
        .flat_map(|l| parse(l.trim()))
        .sum();

    println!("{}", format(result));
}
