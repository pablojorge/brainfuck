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
            use std::io;
            use std::io::prelude::*;
            use std::convert::TryInto;

            macro_rules! print_mem {
                ($mem:expr, $ptr:expr) => ({
                    let aux: u8 = $mem[$ptr].try_into().unwrap();
                    print!("{}", aux as char);
                    io::stdout().flush()?;
                })
            }

            #[allow(unused_macros)]
            macro_rules! read_mem {
                ($mem:expr, $ptr:expr) => ({
                    let mut aux: [u8; 1] = [0];
                    io::stdin().read(&mut aux)?;
                    $mem[$ptr] = aux[0].into();
                })
            }

            fn main() -> Result<(), std::io::Error> {
                let mut mem: Vec<u32> = Vec::with_capacity(30000);
                let mut ptr: usize = 0;

                for _ in 0..30000 {
                    mem.push(0)
                };
        "#)
    }

    fn translate(opcode: char) -> String {
        String::from(
            match opcode {
                '>' => "ptr+=1;",
                '<' => "ptr-=1;",
                '+' => "mem[ptr]+=1;",
                '-' => "mem[ptr]-=1;",
                '.' => "print_mem!(mem, ptr);",
                ',' => "read_mem!(mem, ptr);",
                '[' => "while mem[ptr] > 0 {",
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
