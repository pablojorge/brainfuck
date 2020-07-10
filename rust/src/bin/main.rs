use std::io::{self, prelude::*};
use std::env;
use std::fs::File;

use brainfuck as bf;

fn read_file(filename: &str) -> Result<String, io::Error> {
    let mut contents = String::new();

    File::open(filename)?
        .read_to_string(&mut contents)?;

    return Ok(contents);
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let filename = match args.get(1) {
        Some(s) => s,
        None => panic!("missing filename")
    };

    let contents = read_file(&filename).expect("Error reading file");

    let tokens = bf::tokenize(&contents.chars().collect());

    let expressions = bf::parse(&tokens).expect("Error compiling program");

    let expressions = bf::optimize(&expressions);

    bf::run(&expressions).expect("Error running program");
}
