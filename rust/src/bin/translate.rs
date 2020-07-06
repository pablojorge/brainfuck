use std::io::{self, prelude::*};
use std::env;

use brainfuck as bf;

trait Target {
    fn preamble() -> String;
    fn translate(token: &bf::Expression) -> String;
    fn epilogue() -> String;
}

struct RustTarget;

impl Target for RustTarget {
    fn preamble() -> String {
        String::from(r#"
            use brainfuck as bf;

            fn main() -> Result<(), std::io::Error> {
                let mut mem = bf::Buffer::<u32>::new(30000);
        "#)
    }

    fn translate(expression: &bf::Expression) -> String {
        match expression {
            &bf::Expression::MoveForward(n) => format!("mem.fwd({});", n),
            &bf::Expression::MoveBack(n)    => format!("mem.bwd({});", n),
            &bf::Expression::IncValue(n)    => format!("mem.inc({});", n),
            &bf::Expression::DecValue(n)    => format!("mem.dec({});", n),
             bf::Expression::OutputValue    => format!("bf::print_mem(mem.read())?;"),
             bf::Expression::InputValue     => format!("mem.write(bf::read_mem()?);"),
             bf::Expression::Loop(sub_exp)  => format!(
                "while mem.read() > 0 {{ {} }}",
                do_translate::<Self>(sub_exp)
            ),
        }
    }

    fn epilogue() -> String {
        String::from(r#"
                Ok(())
            }
        "#)
    }
}

struct CTarget;

impl Target for CTarget {
    fn preamble() -> String {
        String::from("
            #include <stdio.h>
            #include <stdlib.h>
            int main() {
                char mem[30000],
                *ptr = mem;
        ")
    }

    fn translate(expression: &bf::Expression) -> String {
        match expression {
            &bf::Expression::MoveForward(n) => format!(" ptr+={};", n),
            &bf::Expression::MoveBack(n)    => format!(" ptr-={};", n),
            &bf::Expression::IncValue(n)    => format!("*ptr+={};", n),
            &bf::Expression::DecValue(n)    => format!("*ptr-={};", n),
             bf::Expression::OutputValue    => format!("putchar(*ptr);"),
             bf::Expression::InputValue     => format!("*ptr = getchar();"),
             bf::Expression::Loop(sub_exp)  => format!(
                "while(*ptr){{ {} }}",
                do_translate::<Self>(sub_exp)
            ),
        }
    }

    fn epilogue() -> String {
        String::from("
                return 0;
            }
        ")
    }
}

fn do_translate<T: Target>(expressions: &Vec<bf::Expression>) -> String {
    let mut ret = String::new();

    for expression in expressions {
        ret.push_str(&T::translate(expression))
    }

    ret
}

fn translate<T: Target>(expressions: &Vec<bf::Expression>) -> String {
    let mut program = String::new();

    program.push_str(&T::preamble());
    program.push_str(&do_translate::<T>(expressions));
    program.push_str(&T::epilogue());

    program
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut bf_input = String::new();
    io::stdin().read_to_string(&mut bf_input).expect("Error reading stdin");

    let tokens = bf::tokenize(&bf_input.chars().collect());
    let expressions = bf::parse(&tokens).expect("Error compiling program");
    let expressions = bf::optimize(&expressions);

    if args.len() < 2 || args[1] == "rs" {
        print!("{}", translate::<RustTarget>(&expressions));
    } else if args[1] == "c" {
        print!("{}", translate::<CTarget>(&expressions));
    } else {
        panic!("{:?}: Unsupported target language", args[1]);
    }
}
