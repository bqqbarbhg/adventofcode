use regex::{Regex, Captures};
use std::vec::Vec;

pub trait FromCapture where Self: Sized {
    fn from_capture_at(cap: &Captures, index: usize) -> Option<Self>;
    fn from_capture(cap: &Captures) -> Option<Self> {
        Self::from_capture_at(cap, 1)
    }
}

impl FromCapture for i32 {
    fn from_capture_at(cap: &Captures, index: usize) -> Option<i32> {
        cap.get(index)?.as_str().parse().ok()
    }
}

impl FromCapture for usize {
    fn from_capture_at(cap: &Captures, index: usize) -> Option<usize> {
        cap.get(index)?.as_str().parse().ok()
    }
}

impl FromCapture for char {
    fn from_capture_at(cap: &Captures, index: usize) -> Option<char> {
        cap.get(index)?.as_str().chars().next()
    }
}

impl FromCapture for String {
    fn from_capture_at(cap: &Captures, index: usize) -> Option<String> {
        Some(cap.get(index)?.as_str().to_string())
    }
}

impl<T1: FromCapture, T2: FromCapture> FromCapture for (T1, T2) {
    fn from_capture_at(cap: &Captures, start: usize) -> Option<(T1, T2)> {
        Some((
            T1::from_capture_at(cap, start + 0)?,
            T2::from_capture_at(cap, start + 1)?,
        ))
    }
}

impl<T1: FromCapture, T2: FromCapture, T3: FromCapture> FromCapture for (T1, T2, T3) {
    fn from_capture_at(cap: &Captures, start: usize) -> Option<(T1, T2, T3)> {
        Some((
            T1::from_capture_at(cap, start + 0)?,
            T2::from_capture_at(cap, start + 1)?,
            T3::from_capture_at(cap, start + 2)?,
        ))
    }
}

pub fn match_capture<T: FromCapture>(re: &Regex, s: &str) -> Option<T> {
    T::from_capture(&re.captures(s)?)
}

pub fn match_iter<'a, T: FromCapture>(re: &'a Regex, s: &'a str) -> impl Iterator<Item=Option<T>> + 'a {
    re.captures_iter(s).map(|c| T::from_capture(&c))
}

pub fn ensure_size_with<T, F>(v: &mut Vec<T>, size: usize, f: F)
    where F: FnMut() -> T {
    if size > v.len() {
        v.resize_with(size, f)
    }
}

pub fn ensure_size<T: Default>(v: &mut Vec<T>, size: usize) {
    ensure_size_with(v, size, T::default);
}