use std::io;
use std::env;
use std::io::prelude::*;

trait Target {
    fn preamble() -> String;
    fn translate(opcode: char) -> String;
    fn epilogue() -> String;
}

struct RustTarget;

impl Target for RustTarget {
    fn preamble() -> String {
        String::from(r#"
            mod bf;

            fn main() -> Result<(), std::io::Error> {
                let mut state = bf::BFState::new(30000);
        "#)
    }

    fn translate(opcode: char) -> String {
        String::from(
            match opcode {
                '>' => "state.fwd();",
                '<' => "state.bwd();",
                '+' => "state.inc();",
                '-' => "state.dec();",
                '.' => "bf::print_mem(state.read())?;",
                ',' => "state.write(bf::read_mem()?);",
                '[' => "while state.read() > 0 {",
                ']' => "}",
                _ => "",
            }
        )
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

    fn translate(opcode: char) -> String {
        String::from(
            match opcode {
                '>' => "++ptr;",
                '<' => "--ptr;",
                '+' => "++(*ptr);",
                '-' => "--(*ptr);",
                '.' => "putchar(*ptr);",
                ',' => "*ptr = getchar();
                        if (*ptr == EOF) exit(0);",
                '[' => "while(*ptr) {",
                ']' => "}",
                _ => "",
            }
        )
    }

    fn epilogue() -> String {
        String::from("
                return 0;
            }
        ")
    }
}

fn translate<T: Target>(bf_input: String) -> String {
    let mut program = String::new();

    program.push_str(&T::preamble());

    for opcode in bf_input.chars() {
        program.push_str(&T::translate(opcode))
    }

    program.push_str(&T::epilogue());

    program
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let mut bf_input = String::new();
    io::stdin().read_to_string(&mut bf_input).expect("Error reading stdin");

    if args.len() < 2 || args[1] == "rust" {
        print!("{}", translate::<RustTarget>(bf_input));
    } else {
        print!("{}", translate::<CTarget>(bf_input));
    }
}
