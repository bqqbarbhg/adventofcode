use std::{mem, collections::HashMap};
use regex::Regex;

#[derive(Default)]
pub struct Dir {
    pub entries: HashMap<String, Entry>,
}

pub struct File {
    pub size: usize,
}

pub enum Entry {
    Dir(Dir),
    File(File),
}

fn re_match(re: &Regex, text: &str) -> Vec<Option<String>> {
    if let Some(caps) = re.captures(text) {
        return caps.iter()
            .skip(1)
            .map(|o| o.map(|m| m.as_str().to_string()))
            .collect();
    }
    Vec::new()
}

pub fn exec_line(root: &mut Dir, path: &mut Vec<String>, line: &str) {
    lazy_static! {
        static ref RE_CD: Regex = Regex::new(r"\$\s+cd\s+(\S+)").unwrap();
        static ref RE_DIR: Regex = Regex::new(r"dir\s+(\S+)").unwrap();
        static ref RE_FILE: Regex = Regex::new(r"(\d+)\s+(\S+)").unwrap();
    }

    let mut cwd = root;
    for segment in path.as_slice() {
        cwd = match cwd.entries.get_mut(segment) {
            Some(Entry::Dir(dir)) => dir,
            _ => panic!("{} not found", segment),
        }
    }

    if let [Some(dir)] = re_match(&RE_CD, line).as_mut_slice() {
        match dir.as_str() {
            ".." => { path.pop(); },
            "/" => { path.clear(); },
            _ => {
                path.push(dir.clone());
                cwd.entries.entry(mem::take(dir))
                    .or_insert_with(|| Entry::Dir(Dir::default()));
            },
        }
    } else if let [Some(name)] = re_match(&RE_DIR, line).as_mut_slice() {
        cwd.entries.entry(mem::take(name))
            .or_insert_with(|| Entry::Dir(Dir::default()));
    } else if let [Some(size), Some(name)] = re_match(&RE_FILE, line).as_mut_slice() {
        cwd.entries.entry(mem::take(name))
            .or_insert_with(|| Entry::File(File{ size: size.parse().unwrap() }));
    }
}

pub fn exec_lines(lines: impl Iterator<Item=String>) -> Dir {
    let mut root = Dir::default();
    let mut path: Vec<String> = Vec::new();
    for line in lines {
        exec_line(&mut root, &mut path, &line);
    }
    root
}

pub fn for_dir_sizes<F: FnMut(usize) -> ()>(dir: &Dir, f: &mut F) -> usize {
    let mut size = 0;
    for entry in dir.entries.values() {
        match entry {
            Entry::Dir(d) => {
                size += for_dir_sizes(d, f);
            },
            Entry::File(f) => {
                size += f.size;
            },
        }
    }
    f(size);
    size
}
