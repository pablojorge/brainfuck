use std;
use std::io::{self, prelude::*};
use std::convert::TryInto;

type Position = usize;

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

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Token {
    ProgramStart,
    ProgramEnd,
    LoopStart(Position),
    LoopEnd(Position),
    IncValue(Position),
    DecValue(Position),
    MoveForward(Position),
    MoveBack(Position),
    InputValue(Position),
    OutputValue(Position)
}

pub fn tokenize(program: &Vec<char>) -> Vec<Token> {
    let mut tokens = Vec::new();

    tokens.push(Token::ProgramStart);

    for (pos, opcode) in program.iter().enumerate() {
        match opcode {
            '[' => tokens.push(Token::LoopStart(pos)),
            ']' => tokens.push(Token::LoopEnd(pos)),
            '>' => tokens.push(Token::MoveForward(pos)),
            '<' => tokens.push(Token::MoveBack(pos)),
            '+' => tokens.push(Token::IncValue(pos)),
            '-' => tokens.push(Token::DecValue(pos)),
            '.' => tokens.push(Token::OutputValue(pos)),
            ',' => tokens.push(Token::InputValue(pos)),
            _ => (),
        }
    }

    tokens.push(Token::ProgramEnd);

    return tokens;
}

#[derive(Debug, PartialEq)]
pub enum Expression {
    IncValue,
    DecValue,
    MoveForward,
    MoveBack,
    InputValue,
    OutputValue,
    Loop(Vec<Expression>),
}

pub fn parse(tokens: &Vec<Token>) 
    -> Result<Vec<Expression>, InvalidProgramError> {
    let (expressions, _) = do_parse(tokens.iter(), 0)?;
    Ok(expressions)
}

fn do_parse(mut tokens: std::slice::Iter<Token>, level: u32) 
    -> Result<(Vec<Expression>, std::slice::Iter<Token>), InvalidProgramError>
{
    let mut expressions = Vec::new();

    loop {
        if let Some(token) = tokens.next() {
            match token {
                Token::LoopStart(_) => {
                    let (sub_exp, next_tok) = do_parse(tokens, level + 1)?;
                    expressions.push(Expression::Loop(sub_exp));
                    tokens = next_tok;
                }
                Token::LoopEnd(pos) =>
                    if level == 0 {
                        return Err(InvalidProgramError::
                                   UnexpectedClosingBracket(*pos))
                    } else {
                        return Ok((expressions, tokens))
                    },
                Token::MoveForward(_) =>
                    expressions.push(Expression::MoveForward),
                Token::MoveBack(_) =>
                    expressions.push(Expression::MoveBack),
                Token::IncValue(_) =>
                    expressions.push(Expression::IncValue),
                Token::DecValue(_) =>
                    expressions.push(Expression::DecValue),
                Token::OutputValue(_) =>
                    expressions.push(Expression::OutputValue),
                Token::InputValue(_) =>
                    expressions.push(Expression::InputValue),
                Token::ProgramStart => (),
                Token::ProgramEnd =>
                    if level > 0 {
                        return Err(InvalidProgramError::
                                   ExcessiveOpeningBrackets(0))
                    }
            }
        } else {
            break;
        }
    }

    Ok((expressions, tokens))
}

pub fn run(expressions: &Vec<Expression>) 
    -> Result<(), BFEvalError> {
    let mut state = BFState::new(30000);

    Ok(do_run(expressions, &mut state)?)
}

fn do_run(expressions: &Vec<Expression>, state: &mut BFState)
    -> Result<(), BFEvalError> {
    for expression in expressions {
        match expression {
            Expression::MoveForward => state.fwd(),
            Expression::MoveBack => state.bwd(),
            Expression::IncValue => state.inc(),
            Expression::DecValue => state.dec(),
            Expression::OutputValue => print_mem(state.read())?,
            Expression::InputValue => state.write(read_mem()?),
            Expression::Loop(sub_exp) => {
                while state.read() > 0 {
                    do_run(sub_exp, state)?;
                }
            }
        }
    }

    Ok(())
}