use std::fs::{self, File};
use std::io::{self, Write};
use std::time::Instant;
use std::path::Path;
use std::process::{Stdio,Command};
use std::str;

fn split_lines(s: &str) -> Vec<String> {
    s.lines()
        .map(|l| l.trim().to_owned())
        .filter(|l| !l.is_empty())
        .collect()
}

fn format_lines(lines: &[String]) -> String {
    match &lines {
        &[] => "(empty)".to_owned(),
        &[line] => line.clone(),
        _ => format!("({} lines)", lines.len()),
    }
}

fn main() {
    let self_exe = std::env::current_exe().unwrap();
    let exe_ext = self_exe
        .extension()
        .and_then(|ext| ext.to_str())
        .map(|ext| String::from(".") + ext)
        .unwrap_or(String::new());
    let self_parent = self_exe.parent().unwrap();

    let mut num_ok = 0;
    let mut num_missing = 0;
    let mut num_tested = 0;

    for day in 1..=25 {
        let day_str = format!("{:02}", day);
        let path = Path::new(&day_str);
        if !path.is_dir() {
            println!("\nTested up to day {}!", day - 1);
            break
        }

        for part in [1,2] {
            let exe_name = format!("day{:02}-part{}{}", day, part, exe_ext);
            let exe_path = self_parent.join(exe_name);
            let input_path = path.join("input.txt");
            let output_path = path.join(format!("output{}.txt", part));

            let input = File::open(input_path).expect("Expected to open input.txt");

            print!("{:02}/{}: ", day, part);
            io::stdout().flush().unwrap();

            let pre = Instant::now();
            let exe_output = Command::new(exe_path.as_path())
                .stdin(Stdio::from(input))
                .output()
                .expect("Failed to run executable");
            let post = Instant::now();

            let result = str::from_utf8(&exe_output.stdout).unwrap();
            let result = split_lines(result);

            if let Some(output) = fs::read_to_string(output_path).ok() {
                let output = split_lines(&output);
                if result == output {
                    let ms = (post - pre).as_millis();
                    let line = format!("{} OK!", format_lines(&result));
                    println!("{:-24} \x1b[90m({}ms)\x1b[0m", line, ms);
                    num_ok += 1;
                } else {
                    println!("FAIL\n-- expected --\n{}\n-- got --\n{}\n--",
                        output.join("\n"), result.join("\n"));
                }
            } else {
                println!("{}", format_lines(&result));
                num_missing += 1;
            }
            num_tested += 1;
        }
    }

    if num_missing > 0 {
        println!("{}/{} OK ({} missing)", num_ok, num_tested, num_missing);
    } else {
        println!("{}/{} OK", num_ok, num_tested);
    }
}
