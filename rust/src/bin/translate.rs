use std::io::{self, prelude::*};
use std::env;

use brainfuck as bf;

trait Target {
    fn translate(token: &bf::Token) -> &'static str;
}

struct RustTarget;

impl Target for RustTarget {
    fn translate(token: &bf::Token) -> &'static str {
        match token {
            bf::Token::ProgramStart   => r#"
                use brainfuck as bf;

                fn main() -> Result<(), std::io::Error> {
                    let mut mem = bf::Buffer::<u32>::new(30000);
            "#,
            bf::Token::MoveForward(_) => "mem.fwd();",
            bf::Token::MoveBack(_)    => "mem.bwd();",
            bf::Token::IncValue(_)    => "mem.inc();",
            bf::Token::DecValue(_)    => "mem.dec();",
            bf::Token::OutputValue(_) => "bf::print_mem(mem.read())?;",
            bf::Token::InputValue(_)  => "mem.write(bf::read_mem()?);",
            bf::Token::LoopStart(_)   => "while mem.read() > 0 {",
            bf::Token::LoopEnd(_)     => "}",
            bf::Token::ProgramEnd     => r#"
                    Ok(())
                }
            "#
        }
    }
}

struct CTarget;

impl Target for CTarget {
    fn translate(token: &bf::Token) -> &'static str {
        match token {
            bf::Token::ProgramStart   => "
                #include <stdio.h>
                #include <stdlib.h>
                int main() {
                    char mem[30000],
                    *ptr = mem;
            ",
            bf::Token::MoveForward(_) => "++ptr;",
            bf::Token::MoveBack(_)    => "--ptr;",
            bf::Token::IncValue(_)    => "++(*ptr);",
            bf::Token::DecValue(_)    => "--(*ptr);",
            bf::Token::OutputValue(_) => "putchar(*ptr);",
            bf::Token::InputValue(_)  => "
                *ptr = getchar();
                if (*ptr == EOF) exit(0);
            ",
            bf::Token::LoopStart(_)   =>  "while(*ptr) {",
            bf::Token::LoopEnd(_)     => "}",
            bf::Token::ProgramEnd     => "
                    return 0;
                }
            "
        }
    }
}

fn translate<T: Target>(tokens: &Vec<bf::Token>) -> String {
    let mut program = String::new();

    for token in tokens {
        program.push_str(&T::translate(token))
    }

    program
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut bf_input = String::new();
    io::stdin().read_to_string(&mut bf_input).expect("Error reading stdin");

    let tokens = bf::tokenize(&bf_input.chars().collect());

    if args.len() < 2 || args[1] == "rs" {
        print!("{}", translate::<RustTarget>(&tokens));
    } else {
        print!("{}", translate::<CTarget>(&tokens));
    }
}
