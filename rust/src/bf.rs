use std;
use std::io::{self, prelude::*};
use std::collections::HashMap;
use std::convert::TryInto;

type Program = [u8];
type Position = usize;
type JumpMap = HashMap<Position, Position>;

#[derive(Debug)]
pub enum InvalidProgramError {
    ExcessiveOpeningBrackets(Position),
    UnexpectedClosingBracket(Position),
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
    pub fn new(mem_size: usize) -> Self {
        let mut state = Self {
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

pub fn read_mem() -> Result<MemElem, std::io::Error> {
    let mut input: [u8; 1] = [0];
    io::stdin().read(&mut input)?;
    Ok(input[0].into())
}

pub fn print_mem(mem: MemElem) -> Result<(), std::io::Error> {
    let x: u8 = mem.try_into().unwrap();
    print!("{}", x as char);
    io::stdout().flush()
}

pub struct Interpreter <'a> {
    program: &'a [u8],
    state: BFState,
    jumps: JumpMap
}

impl <'a> Interpreter <'a> {
    pub fn new(program: &'a [u8], mem_size: usize) -> Result<Self, BFEvalError> {
        Ok(Self {
            program,
            state: BFState::new(mem_size),
            jumps: Self::_find_jumps(program)?
        })
    }

    fn _loop(program: &Program,
             handler: &mut dyn FnMut(char, Position) -> Result<Position, BFEvalError>) 
            -> Result<(), BFEvalError> {
        let plen = program.len();
        let mut pos: Position = 0;

        while pos < plen {
            let opcode = program[pos] as char;
            pos = handler(opcode, pos)?;
        }

        Ok(())
    }

    fn _find_jumps(program: &[u8]) -> Result<JumpMap, BFEvalError> {
        let mut stack: Vec<usize> = Vec::new();
        let mut jumps = JumpMap::new();

        Self::_loop(
            program,
            &mut |opcode, pc| -> Result<Position, BFEvalError> { 
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
                };
                Ok(pc+1)
            }
        )?;

        match stack.pop() {
            Some(n) => Err(
                BFEvalError::InvalidProgramError(
                    InvalidProgramError::ExcessiveOpeningBrackets(n)
                )
            ),
            None => Ok(jumps)
        }
    }

    pub fn run(&mut self) -> Result<(), BFEvalError> {
        Self::_loop(
            self.program,
            &mut |opcode, pc| -> Result<Position, BFEvalError> { 
                let ret = match opcode {
                    '[' => if self.state.read() == 0 {self.jumps[&pc]} else {pc+1},
                    ']' => if self.state.read() != 0 {self.jumps[&pc]} else {pc+1},
                    _ => {
                        match opcode {
                            '>' => self.state.fwd(),
                            '<' => self.state.bwd(),
                            '+' => self.state.inc(),
                            '-' => self.state.dec(),
                            '.' => print_mem(self.state.read())?,
                            ',' => self.state.write(read_mem()?),
                            _ => (),
                        };
                        pc+1
                    }
                };
                Ok(ret)
            }
        )
    }
}
