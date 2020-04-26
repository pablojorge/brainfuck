use std;
use std::io;
use std::io::prelude::*;
use std::collections::HashMap;
use std::convert::TryInto;

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

#[derive(Debug)]
pub enum BFEvalError {
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

type MemElem = u32;
type Memory = Vec<MemElem>;

pub struct BFState {
    mem: Memory,
    ptr: usize,
}

impl BFState {
    pub fn new(mem_size: usize) -> BFState {
        let mut state = BFState {
            mem: Vec::with_capacity(mem_size),
            ptr: 0 
        };

        for _ in 0..mem_size {
            state.mem.push(0)
        };

        state
    }

    pub fn fwd(&mut self) {
        self.ptr += 1;
    }

    pub fn bwd(&mut self) {
        self.ptr -= 1;
    }

    pub fn inc(&mut self) {
        self.mem[self.ptr] += 1;
    }

    pub fn dec(&mut self) {
        self.mem[self.ptr] -= 1;
    }

    pub fn read(&self) -> MemElem {
        self.mem[self.ptr]
    }

    pub fn write(&mut self, val: MemElem) {
        self.mem[self.ptr] = val;
    }
}

pub fn read_mem() -> Result<MemElem, BFEvalError> {
    let mut input: [u8; 1] = [0];
    io::stdin().read(&mut input)?;
    Ok(input[0].into())
}

pub fn print_mem(mem: MemElem) -> Result<(), std::io::Error> {
    let x: u8 = mem.try_into().unwrap();
    print!("{}", x as char);
    io::stdout().flush()
}

pub fn bf_eval(program: &[u8], mem_size: usize) -> Result<(),
                                                          BFEvalError> {
    let mut state = BFState::new(mem_size);
    let jumps = bf_jumps(program)?;

    bf_loop!(opcode, pc, program,
        match opcode {
            '>' => state.fwd(),
            '<' => state.bwd(),
            '+' => state.inc(),
            '-' => state.dec(),
            '.' => print_mem(state.read())?,
            ',' => state.write(read_mem()?),
            '[' => if state.read() == 0 {pc = jumps[&pc]},
            ']' => if state.read() != 0 {pc = jumps[&pc]},
            _ => (),
        }
    );

    Ok(())
}
