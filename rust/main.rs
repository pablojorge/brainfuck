use std::io;
use std::env;
use std::fs::File;
use std::io::prelude::*;

mod brainfuck;

fn read_file(filename: &String) -> Result<String, io::Error> {
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
    let program = program.as_bytes();

    brainfuck::bf_eval(program, 30000).expect("Error running program");
}