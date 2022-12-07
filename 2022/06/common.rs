
pub fn find_unique(data: &[u8], length: usize) -> Option<usize> {
    let mut window = [0isize; 256];
    let mut window_count = 0;

    let mut inc = |byte: u8, delta: isize| -> isize {
        let count = &mut window[byte as usize];
        *count += delta;
        *count
    };

    for (ix, &byte) in data.iter().enumerate() {
        if ix >= length && inc(data[ix - length], -1) == 0 {
            window_count -= 1;
        }
        if inc(byte, 1) == 1 {
            window_count += 1;
        }
        if window_count == length {
            return Some(ix + 1);
        }
    }

    None
}
