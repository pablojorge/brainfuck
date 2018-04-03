use std;
use std::io;
use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::collections::HashMap;

fn read_file(filename: &String) -> Result<String, io::Error> {
    let mut contents = String::new();

    File::open(filename)?
        .read_to_string(&mut contents)?;

    return Ok(contents);
}

macro_rules! bf_loop {
    ($op_name:ident, $pc_name:ident, $program:expr, $body:expr) => ({
        let plen = $program.len();
        let mut $pc_name: usize = 0;

        while $pc_name < plen {
            let $op_name = $program[$pc_name] as char;
            $body;
            $pc_name += 1;
        }
    })
}

#[derive(Debug)]
pub enum InvalidProgramError {
    ExcessiveOpeningBrackets(usize),
    UnexpectedClosingBracket(usize),
}

pub fn bf_jumps(program: &[u8]) -> Result<HashMap<usize, usize>,
                                      InvalidProgramError> {
    let mut stack: Vec<usize> = Vec::new();
    let mut jumps = HashMap::new();

    bf_loop!(opcode, pc, program,
        match opcode {
            '[' => stack.push(pc),
            ']' => {
                let target = match stack.pop() {
                    Some(n) => Ok(n),
                    None => Err(
                        InvalidProgramError::
                        UnexpectedClosingBracket(pc)
                    )
                }?;
                jumps.insert(pc, target);
                jumps.insert(target, pc);
            }
            _ => ()
        }
    );

    match stack.pop() {
        Some(n) => Err(
            InvalidProgramError::
            ExcessiveOpeningBrackets(n)
        ),
        _ => Ok(jumps)
    }
}

macro_rules! print_mem {
    ($mem:expr, $ptr:expr) => ({
        print!("{}", $mem[$ptr] as char);
        io::stdout().flush()?
    })
}

macro_rules! read_mem {
    ($mem:expr, $ptr:expr) => ({
        io::stdin().read(&mut $mem[$ptr..$ptr+1])?;
    })
}

#[derive(Debug)]
enum BFEvalError {
    InvalidProgramError(InvalidProgramError),
    IOError(std::io::Error),
}

impl std::convert::From<std::io::Error> for BFEvalError {
    fn from(err: std::io::Error) -> BFEvalError {
        BFEvalError::IOError(err)
    }
}

impl std::convert::From<InvalidProgramError> for BFEvalError {
    fn from(err: InvalidProgramError) -> BFEvalError {
        BFEvalError::InvalidProgramError(err)
    }
}

fn bf_eval(program: &[u8]) -> Result<(), BFEvalError> {
    let mut mem: [u8; 30000] = [0; 30000];
    let mut ptr: usize = 0;
    let jumps = bf_jumps(program)?;

    bf_loop!(opcode, pc, program,
        match opcode {
            '>' => ptr+=1,
            '<' => ptr-=1,
            '+' => mem[ptr]+=1,
            '-' => mem[ptr]-=1,
            '.' => print_mem!(mem, ptr),
            ',' => read_mem!(mem, ptr),
            '[' => if mem[ptr] == 0 {pc = jumps[&pc]},
            ']' => if mem[ptr] != 0 {pc = jumps[&pc]},
            _ => (),
        }
    );

    Ok(())
}

fn main() {
    let args: Vec<String> = env::args().collect();

    if args.len() < 2 {
        panic!("missing filename");
    }

    let program = read_file(&args[1]).expect("Error reading file");
    let program = program.as_bytes();

    bf_eval(program).expect("Error running program");
}