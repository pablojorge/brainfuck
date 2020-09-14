#![feature(asm)]

use std::io::{prelude::*};
use std::env;
use std::fs::File;
use std::convert::TryInto;

use brainfuck as bf;

extern crate assembler;

use ::assembler::{
    ExecutableAnonymousMemoryMap,
    InstructionStream,
    InstructionStreamHints
};

fn compile(expressions: &Vec<bf::Expression>, stream: &mut InstructionStream) {
    for expression in expressions {
        match expression {
            // 000000000000000c forward:
            //        c: 48 81 c6 80 00 00 00          addq    $128, %rsi
            &bf::Expression::MoveForward(n) => {
                stream.emit_bytes(b"\x48\x81\xc6");
                stream.emit_double_word((n*4).try_into().unwrap());
            },
            // 0000000000000013 backward:
            //       13: 48 81 ee 80 00 00 00          subq    $128, %rsi
            &bf::Expression::MoveBack(n) => {
                stream.emit_bytes(b"\x48\x81\xee");
                stream.emit_double_word((n*4).try_into().unwrap());
            },
            // 0000000000000000 increment:
            //        0: 81 06 80 00 00 00             addl    $128, (%rsi)
            &bf::Expression::IncValue(n) => {
                stream.emit_bytes(b"\x81\x06");
                stream.emit_double_word(n);
            },
            // 0000000000000006 decrement:
            //        6: 81 2e 80 00 00 00             subl    $128, (%rsi)
            &bf::Expression::DecValue(n) => {
                stream.emit_bytes(b"\x81\x2e");
                stream.emit_double_word(n);
            },
            // 0000000000000031 write:
            //       31: 48 c7 c0 04 00 00 02          movq    $33554436, %rax
            //       38: 48 c7 c7 01 00 00 00          movq    $1, %rdi
            //       3f: 48 c7 c2 01 00 00 00          movq    $1, %rdx
            //       46: 0f 05                         syscall
             bf::Expression::OutputValue => {
                stream.emit_bytes(b"\x48\xc7\xc0");
                stream.emit_double_word(0x02000004);
                stream.emit_bytes(b"\x48\xc7\xc7\x01\x00\x00\x00");
                stream.emit_bytes(b"\x48\xc7\xc2\x01\x00\x00\x00");
                stream.emit_bytes(b"\x0f\x05");
             },
            // 000000000000001a read:
            //       1a: 48 c7 c0 03 00 00 02          movq    $33554435, %rax
            //       21: 48 c7 c7 00 00 00 00          movq    $0, %rdi
            //       28: 48 c7 c2 01 00 00 00          movq    $1, %rdx
            //       2f: 0f 05                         syscall
             bf::Expression::InputValue => {
                stream.emit_bytes(b"\x48\xc7\xc0");
                stream.emit_double_word(0x02000003);
                stream.emit_bytes(b"\x48\xc7\xc7\x00\x00\x00\x00");
                stream.emit_bytes(b"\x48\xc7\xc2\x01\x00\x00\x00");
                stream.emit_bytes(b"\x0f\x05");
             },
             bf::Expression::Loop(sub_exp) => {
                let loop_start = stream.create_label();
                let post_loop = stream.create_label();

                //   48: 83 3e 00                      cmpl    $0, (%rsi)
                //   4b: 0f 84 00 00 00 00             je  0 <loop_end>
                stream.emit_bytes(b"\x83\x3e\x00");
                stream.jz_Label_1(post_loop);       // --*
                stream.attach_label(loop_start);    // <-|-*
                                                    //   | |
                compile(sub_exp, stream);           //   | |
                                                    //   | |
                //   51: 83 3e 00  cmpl $0, (%rsi)  //   | |
                //   54: 0f 85 00 00 00 00  jne 0   //   | |
                stream.emit_bytes(b"\x83\x3e\x00"); //   | |
                stream.jnz_Label_1(loop_start);     // --|-*
                stream.attach_label(post_loop);     // <-*
            }
        }
    }
}

fn opcodes_size(stats: &bf::Stats) -> usize {
    return     7 * stats.fwd_count +
               7 * stats.bwd_count +
               6 * stats.inc_count +
               6 * stats.dec_count +
              23 * stats.output_count +
              23 * stats.input_count + 
           9 * 2 * stats.loop_count;
}

fn run(expressions: &Vec<bf::Expression>) {
    let stats = bf::stats(expressions);

    let mem_size = opcodes_size(&stats) 
                    + 1  // retq
                    + 8; // stream buffer

    let mut memory_map = ExecutableAnonymousMemoryMap::new(mem_size,
                                                           false,
                                                           false).unwrap();
    let mut instruction_stream = memory_map.instruction_stream(
        &InstructionStreamHints {
            number_of_labels: stats.loop_count * 2,
            number_of_8_bit_jumps: 0,
            number_of_32_bit_jumps: stats.loop_count * 2,
            number_of_emitted_labels: 0
        }
    );

    // capture the address where the instructions start:
    let function_pointer = instruction_stream.nullary_function_pointer::<()>();

    // transform all the expressions into opcodes:
    compile(expressions, &mut instruction_stream);

    // 000000000000005b finish:
    //       5b: c3                            retq
    instruction_stream.emit_byte(0xc3);

    // resolve jumps and make memory executable:
    instruction_stream.finish();

    // prepare working buffer:
    let memory = [0u32; 30000];

    unsafe {
        // point RSI to the buffer:
        asm!(
            "movq {mem}, %rsi",
            mem = in (reg) &memory,
            options(att_syntax)
        );

        // jump to the generated code:
        function_pointer();
    }
}

fn main() {
    let args: Vec<String> = env::args().collect();

    let filename = match args.get(1) {
        Some(s) => s,
        None => panic!("missing filename")
    };

    let mut contents = String::new();

    let mut file = File::open(filename).expect("Error opening file");
    file.read_to_string(&mut contents).expect("Error reading file");

    let tokens = bf::tokenize(&contents.chars().collect());
    let expressions = bf::parse(&tokens).expect("Error compiling program");
    let expressions = bf::optimize(&expressions);

    run(&expressions);
}
