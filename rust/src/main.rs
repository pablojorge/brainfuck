use std::io::{self, prelude::*};
use std::env;
use std::fs::File;

mod bf;

fn read_file(filename: &str) -> Result<String, io::Error> {
    let mut contents = String::new();

    File::open(filename)?
        .read_to_string(&mut contents)?;

    return Ok(contents);
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        panic!("missing filename");
    }

    let program = read_file(&args[1]).expect("Error reading file");

    let mut interpreter = bf::Interpreter::new(
        program.as_bytes(),
        30000
    ).expect("Error initializing interpreter");

    interpreter.run().expect("Error running program");
}
