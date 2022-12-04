use std::fs::{self, File};
use std::io::{self, Write};
use std::path::Path;
use std::process::{Stdio,Command};
use std::str;

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

    for day in 1..=24 {
        let day_str = format!("{:02}", day);
        let path = Path::new(&day_str);
        if !path.is_dir() {
            println!("Tested up to day {}!", day - 1);
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

            let exe_output = Command::new(exe_path.as_path())
                .stdin(Stdio::from(input))
                .output()
                .expect("Failed to run executable");

            let result = str::from_utf8(&exe_output.stdout).unwrap();

            let result = result.trim();

            if let Some(output) = fs::read_to_string(output_path).ok() {
                let output = output.trim();
                if result == output {
                    println!("{} OK!", result);
                    num_ok += 1;
                } else {
                    println!("{} FAIL (expected {})", result, output);
                }
            } else {
                println!("{}", result);
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
