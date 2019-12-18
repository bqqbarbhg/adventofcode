use std::io::prelude::*;

fn fuel_for_mass(mass: i32) -> i32 {
    let fuel = mass / 3 - 2;
    if fuel > 0 {
        fuel + fuel_for_mass(fuel)
    } else {
        0
    }
}

fn main() {
    let stdin = std::io::stdin();
    let sum: i32 = stdin.lock().lines()
        .flatten()
        .flat_map(|l| l.parse::<i32>())
        .map(fuel_for_mass)
        .sum();
    println!("{}\n", sum);
}

