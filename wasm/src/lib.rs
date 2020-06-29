use wasm_bindgen::prelude::*;

#[cfg(feature = "wee_alloc")]
#[global_allocator]
static ALLOC: wee_alloc::WeeAlloc = wee_alloc::WeeAlloc::INIT;

use std;
use std::fmt;
use std::collections::HashMap;
use std::convert::TryInto;
extern crate num;

use std::ops::{AddAssign, SubAssign};
use num::{Zero,One};

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

pub struct Buffer<T> {
    buf: Vec<T>,
    ptr: usize,
}

impl<T> Buffer<T> 
	where T: Zero + One + Copy + AddAssign + SubAssign {
    pub fn new(buf_size: usize) -> Self {
        let mut buffer = Self {
            buf: Vec::with_capacity(buf_size),
            ptr: 0 
        };

        for _ in 0..buf_size {
            buffer.buf.push(T::zero())
        };

        buffer
    }

    pub fn clone(buf: &[T]) -> Self {
        let mut buffer = Self {
            buf: Vec::with_capacity(buf.len()),
            ptr: 0 
        };

        for i in 0..buf.len() {
            buffer.buf.push(buf[i]);
        };

        buffer
    }

    pub fn len(&mut self) -> usize {
        self.buf.len()
    }

    pub fn fwd(&mut self) {
        self.ptr += 1;
    }

    pub fn bwd(&mut self) {
        self.ptr -= 1;
    }

    pub fn inc(&mut self) {
        self.buf[self.ptr] += T::one();
    }

    pub fn dec(&mut self) {
        self.buf[self.ptr] -= T::one();
    }

    pub fn read(&self) -> T {
        self.buf[self.ptr]
    }

    pub fn write(&mut self, val: T) {
        self.buf[self.ptr] = val;
    }
}

#[wasm_bindgen]
pub struct Interpreter {
    program: Buffer<u8>,
    jumps: JumpMap,
    memory: Buffer<u32>,
    input: Buffer<u8>,
    output: Buffer<u8>,
}

#[wasm_bindgen]
impl Interpreter {
    pub fn new(program: &str, input: &str, mem_size: usize) -> Self {
        Self {
            program: Buffer::clone(program.as_bytes()),
            jumps: Self::_find_jumps(program.as_bytes()),
            memory: Buffer::new(mem_size),
            input: Buffer::clone(input.as_bytes()),
            output: Buffer::new(mem_size),
        }
    }

    fn _find_jumps(program: &[u8]) -> JumpMap {
        let mut stack: Vec<usize> = Vec::new();
        let mut jumps = JumpMap::new();
        let plen = program.len();
        let mut pos: Position = 0;

        while pos < plen {
            let opcode = program[pos] as char;
            match opcode {
                '[' => stack.push(pos),
                ']' => {
                    let target = match stack.pop() {
                        Some(n) => Ok(n),
                        None => Err(
                            InvalidProgramError::
                            UnexpectedClosingBracket(pos)
                        )
                    }.unwrap();
                    jumps.insert(pos, target);
                    jumps.insert(target, pos);
                }
                _ => ()
            };
            pos += 1;
        };

        match stack.pop() {
            Some(n) => Err(
                BFEvalError::InvalidProgramError(
                    InvalidProgramError::ExcessiveOpeningBrackets(n)
                )
            ),
            None => Ok(jumps)
        }.unwrap()
    }

    pub fn tick(&mut self) -> bool {
    	let mut _yield: bool = false;

        while !_yield && self.program.ptr < self.program.len() {
        	let opcode: u8 = self.program.read().into();
        	let opcode = opcode as char;
            match opcode {
	            '>' => self.memory.fwd(),
	            '<' => self.memory.bwd(),
	            '+' => self.memory.inc(),
	            '-' => self.memory.dec(),
                '[' => if self.memory.read() == 0 {
                	self.program.ptr = self.jumps[&self.program.ptr];
                	continue;
                },
                ']' => if self.memory.read() != 0 {
                	self.program.ptr = self.jumps[&self.program.ptr];
                	continue;
                },
	            ',' => {
	            	if self.input.ptr == self.input.len() {
	            		return true;
	            	}
	            	self.memory.write(self.input.read().into());
	            	self.input.fwd();
	            },
	            '.' => {
	            	self.output.write(self.memory.read().try_into().unwrap());
	            	if self.memory.read() == 10 {
	            		_yield = true
	            	};
	            	self.output.fwd();
	            },
	            _ => (),
            }
            self.program.ptr += 1;
        }

        !_yield
    }

    pub fn render(&self) -> String {
        self.to_string()
    }
}

impl fmt::Display for Interpreter {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for i in 0..self.output.ptr {
            write!(f, "{}", self.output.buf[i] as char)?
        };

        Ok(())
    }
}
