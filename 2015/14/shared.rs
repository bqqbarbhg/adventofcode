
macro_rules! try_some {
	($e:expr) => (match $e {
		Some(value) => value,
		None => return None,
	})
}

enum PatternPart<'a> {
	Literal(&'a str),
	Capture,
}

impl<'a> PatternPart<'a> {
	fn parse(token: &'a str) -> PatternPart {
		match token {
		"%" => PatternPart::Capture,
		"%%" => PatternPart::Literal("%"),
		other => PatternPart::Literal(other),
		}
	}
}

pub struct Pattern<'a> {
	parts: Vec<PatternPart<'a>>,
	capture_count: usize,
}

impl<'a> Pattern<'a> {
	pub fn parse(statement: &'a str) -> Pattern<'a> {
		let parts: Vec<PatternPart<'a>> = statement
			.split(char::is_whitespace)
			.map(PatternPart::parse)
			.collect();

		let capture_count = parts.iter()
			.filter(|part| match part {
				&&PatternPart::Capture => true,
				_ => false
			})
			.count();
		
		return Pattern {
			parts: parts,
			capture_count: capture_count,
		};
	}
}

pub fn pattern_match<'a, 'b>(pattern: Pattern<'a>, s: &'b str) -> Option<Vec<&'b str>> {

	let mut matches = Vec::with_capacity(pattern.capture_count);

	let mut count = 0;

	let tokens = s.split(char::is_whitespace);
	for (pat, tok) in pattern.parts.iter().zip(tokens) {
		match pat {
		&PatternPart::Literal(lit) => if lit != tok { return None },
		&PatternPart::Capture => matches.push(tok),
		}
		
		count += 1;
	}

	if count < pattern.parts.len() { return None }
	return Some(matches)
}

#[derive(Debug)]
pub struct Reindeer {
	pub name: String,
	pub fly_speed: i32,
	pub fly_time: i32,
	pub rest_time: i32,
}

impl Reindeer {
	pub fn from_line(line: &str) -> Option<Reindeer> {
		let pattern_str = "% can fly % km/s for % seconds, but then must rest for % seconds.";
		let pattern = Pattern::parse(pattern_str);

		let matches = try_some!(pattern_match(pattern, line));

		return Some(Reindeer {
			name: matches[0].to_string(),
			fly_speed: try_some!(matches[1].parse().ok()),
			fly_time: try_some!(matches[2].parse().ok()),
			rest_time: try_some!(matches[3].parse().ok()),
		});
	}
}

