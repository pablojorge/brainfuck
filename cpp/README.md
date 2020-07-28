## C++-land

This directory contains the sources of the interpreters and the JIT compiler written in C++.

See a discussion of the JIT compiler [here](https://pablojorge.github.io/blog/2020/07/27/bf-jit-compiler-in-cpp.html).

The original C++ version was based on the [Rust](../rust/) version, so it has a more "ADT" style, with enums representing the various token types and expressions. Then a more traditional, OOP-style version with rutime polymorphism was added, and finally, the JIT-version based on the classes written for the OOP.

The full source of the ADT version is in [brainfuck-adt.cpp](./brainfuck-adt.cpp)

The reusable classes for the OOP version are in [brainfuck.h](./brainfuck.h), and the main interpreter is in [brainfuck-oop.cpp](./brainfuck-oop.cpp)

Finally, the JIT version is in [brainfuck-jit.cpp](./brainfuck-jit.cpp).

Just run `make` inside this directory to build them all:

```
$ make CXX=g++-10
g++-10 -std=c++14 -g -O3 brainfuck-adt.cpp -o brainfuck-adt
g++-10 -std=c++14 -g -O3 brainfuck-jit.cpp -o brainfuck-jit
g++-10 -std=c++14 -g -O3 brainfuck-oop.cpp -o brainfuck-oop
```

Additionally, to assist in the creation of the JIT version, there's a complementary asm source used to extract the opcodes: [brainfuck.s](./brainfuck.s):

```
$ make dump
as -arch x86_64 brainfuck.s -o brainfuck.o
objdump -dS ./brainfuck.o

./brainfuck.o:	file format Mach-O 64-bit x86-64

Disassembly of section __TEXT,__text:

0000000000000000 increment:
       0: 48 c7 c0 01 00 00 00         	movq	$1, %rax
       7: 01 07                        	addl	%eax, (%rdi)
[...]
0000000000000070 break:
      70: cc                           	int3
0000000000000071 finish:
      71: c3                           	retq
```