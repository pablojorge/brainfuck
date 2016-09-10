# Brainfuck Experiments

This project contains several interpreters for the [brainfuck language](http://www.muppetlabs.com/~breadbox/bf/). Currently, this is what's available:

 * [python/brainfuck.py](python/brainfuck.py): Interpreter in Python
 * [python/brainfuck-simple.py](python/brainfuck-simple.py): Faster version of the Python interpreter
 * [python/brainfuck-rpython.py](python/brainfuck-rpython.py): RPython-compatible version of the Python interpreter
 * [c/brainfuck.c](c/brainfuck.c): Interpreter in C
 * [haskell/brainfuck.hs](haskell/brainfuck.hs): Interpreter in Haskell
 * [haskell/bf2c.hs](haskell/bf2c.hs): Translator from brainfuck to C in Haskell
 * [asm/brainfuck.s](asm/brainfuck.s): Interpreter in assembler for x86_64
 * [javascript/brainfuck.js](javascript/brainfuck.js): Interpreter in Javascript (live version [here](http://pablojorge.github.io/brainfuck))
 * [go/brainfuck.go](go/brainfuck.go): Interpreter in Go
 * [lua/brainfuck.lua](lua/brainfuck.lua): Interpreter in Lua
 
Is also includes a series of sample programs (further contributions welcome):
 * [hello.bf](programs/hello.bf): Simple hello world! program
 * [primes.bf](programs/primes.bf): Prime number generator. It prompts a number and generates all the primes from 1 up to that number.
 * [rot13.bf](programs/rot13.bf): Applies ROT13 to its input
 * [fibonacci.bf](programs/fibonacci.bf): Fibonacci number generator.
 * [mandelbrot.bf](programs/mandelbrot.bf): Mandelbrot set generator (taken from [http://esoteric.sange.fi/brainfuck/bf-source/prog/mandelbrot.b](http://esoteric.sange.fi/brainfuck/bf-source/prog/mandelbrot.b))
 * [programs/sierpinski.bf](programs/sierpinski.bf): Sierpinsky Triangle generator (taken from the [Spanish Wikipedia article of Brainfuck](http://es.wikipedia.org/wiki/Brainfuck))
 * [cat.bf](programs/cat.bf): Emulates the "cat" program. 
 * [cat2.bf](programs/cat2.bf): An alternative "cat" implementation. 
 * [tolower.bf](programs/tolower.bf): Prints the lower case equivalent of its input, but it's no so smart since it doesn't check for the original case or whether it's a letter or not.
 * [666.bf](programs/666.bf): Prints `666`
 * [random.bf](programs/random.bf): A pseudo random number generator
 * [wc.bf](programs/wc.bf): Word counter (like the standard Unix utility)
 * [dbfi.bf](programs/dbfi.bf): A Brainfuck interpreter in Brainfuck
 * [dbf2c.bf](programs/dbf2c.bf): A Brainfuck to C translator
 * [numwarp.bf](programs/numwarp.bf): Displays numbers from stdin vertically
 * [atoi.bf](programs/atoi.bf): Converts strings to integers
 * [bizzfuzz.bf](programs/bizzfuzz.bf): A Brainfuck implementation of the _Fizzbuzz_ challenge

# System support

All programs were tried in Ubuntu 12.04 and Mac OS X 10.8, except for the Assembler interpreter which only works with the Mac OS X assembler.

# Interpreters

## Python

Using the python interpreter to run the "helloworld" program found in the [Wikipedia article about Brainfuck](http://en.wikipedia.org/wiki/Brainfuck):

    $ cd python
    $ cat << EOF | python brainfuck.py 
    > ++++++++++[>+++++++>++++++++++>+++>+<<<<-]>++.>+.+++++++..+++.>++.<<+++++++++++++++.>.+++.------.--------.>+.>.
    > EOF
    Hello World!

## C

Running the Sierpinsky Triangle generator:

    $ cd c
    $ make brainfuck
    cc brainfuck.c -o brainfuck
    $ ./brainfuck ../samples/sierpinski.bf 
                                    *    
                                   * *    
                                  *   *    
                                 * * * *    
                                *       *    
                               * *     * *    
                              *   *   *   *    
                             * * * * * * * *    
                            *               *    
                           * *             * *    
                          *   *           *   *    
                         * * * *         * * * *    
                        *       *       *       *    
                       * *     * *     * *     * *    
                      *   *   *   *   *   *   *   *    
                     * * * * * * * * * * * * * * * *    
                    *                               *    
                   * *                             * *    
                  *   *                           *   *    
                 * * * *                         * * * *    
                *       *                       *       *    
               * *     * *                     * *     * *    
              *   *   *   *                   *   *   *   *    
             * * * * * * * *                 * * * * * * * *    
            *               *               *               *    
           * *             * *             * *             * *    
          *   *           *   *           *   *           *   *    
         * * * *         * * * *         * * * *         * * * *    
        *       *       *       *       *       *       *       *    
       * *     * *     * *     * *     * *     * *     * *     * *    
      *   *   *   *   *   *   *   *   *   *   *   *   *   *   *   *    
     * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *    

## Haskell

To use the haskell interpreter:

    $ cd haskell
    $ runhaskell brainfuck.hs ../samples/hello.bf 
	Hello World!

## Brainfuck to C translator

Running the same program, but the version translated to C:

    $ cd haskell
    $ make ../samples/sierpinski.c
	runhaskell bf2c.hs < ../samples/sierpinski.bf | indent > sierpinski.c
    $ make sierpinski
	cc sierpinski.c -o sierpinski
    $ ./sierpinski
    [...]

## Assembler

Running the primes generator with the assembler interpreter:

    $ cd asm
    $ make
	as -arch x86_64 brainfuck.s -o brainfuck.o
	ld -e _main -arch x86_64 -lc brainfuck.o -o brainfuck 
	ld: warning: -macosx_version_min not specified, assuming 10.6
	rm brainfuck.o
	$ ./brainfuck ../samples/primes.bf 
	Primes up to: 50
	2 3 5 7 11 13 17 19 23 29 31 37 41 43 47  

## Javascript

There's a live version of the Javascript interpreter at [http://pablojorge.github.io/brainfuck](http://pablojorge.github.io/brainfuck)

It features a debugger-like interface, with support for:
 * Memory inspection
 * Step-by-step execution
 * Modification of program AND input between steps
 * Configurable speed (operations/instructions per cycle, and delay between cycles)

## Golang

Running the number warper with the go interpreter:

    $ cd go
    $ go build -o brainfuck
    $ ./brainfuck ../programs/numwarp.bf 
    32
      /\
       / 
    /\ \/
     /\
      /

## Lua

To use the [Lua](http://www.lua.org/) interpreter:

    $ cd lua
    $ lua brainfuck.lua ../programs/hello.bf
	Hello World!

This interpreter is compatible with Lua 5.1, 5.2 and 5.3 languages,
and runs fast with [LuaJIT](http://luajit.org/).

# Benchmarks

A good program to use as benchmark is the Mandelbrot set generator.

First, with the python interpreter:

    $ time python brainfuck.py ../samples/mandelbrot.bf
    AAAAAAAAAAAAAAAABBBBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDEGFFEEEEDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAAAAABBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDEEEFGIIGFFEEEDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAAABBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDEEEEFFFI KHGGGHGEDDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAABBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEEFFGHIMTKLZOGFEEDDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAABBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEEEFGGHHIKPPKIHGFFEEEDDDDDDDDDCCCCCCCCCCBBBBBBBBBBBBBBBBBB
    AAAAAAAAAABBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEEFFGHIJKS  X KHHGFEEEEEDDDDDDDDDCCCCCCCCCCBBBBBBBBBBBBBBBB
    AAAAAAAAABBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEEFFGQPUVOTY   ZQL[MHFEEEEEEEDDDDDDDCCCCCCCCCCCBBBBBBBBBBBBBB
    AAAAAAAABBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEFFFFFGGHJLZ         UKHGFFEEEEEEEEDDDDDCCCCCCCCCCCCBBBBBBBBBBBB
    AAAAAAABBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEFFFFFFGGGGHIKP           KHHGGFFFFEEEEEEDDDDDCCCCCCCCCCCBBBBBBBBBBB
    AAAAAAABBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDEEEEEFGGHIIHHHHHIIIJKMR        VMKJIHHHGFFFFFFGSGEDDDDCCCCCCCCCCCCBBBBBBBBB
    AAAAAABBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDEEEEEEFFGHK   MKJIJO  N R  X      YUSR PLV LHHHGGHIOJGFEDDDCCCCCCCCCCCCBBBBBBBB
    AAAAABBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDEEEEEEEEEFFFFGH O    TN S                       NKJKR LLQMNHEEDDDCCCCCCCCCCCCBBBBBBB
    AAAAABBCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDEEEEEEEEEEEEFFFFFGHHIN                                 Q     UMWGEEEDDDCCCCCCCCCCCCBBBBBB
    AAAABBCCCCCCCCCCCCCCCCCCCCCCCCCDDDDEEEEEEEEEEEEEEEFFFFFFGHIJKLOT                                     [JGFFEEEDDCCCCCCCCCCCCCBBBBB
    AAAABCCCCCCCCCCCCCCCCCCCCCCDDDDEEEEEEEEEEEEEEEEFFFFFFGGHYV RQU                                     QMJHGGFEEEDDDCCCCCCCCCCCCCBBBB
    AAABCCCCCCCCCCCCCCCCCDDDDDDDEEFJIHFFFFFFFFFFFFFFGGGGGGHIJN                                            JHHGFEEDDDDCCCCCCCCCCCCCBBB
    AAABCCCCCCCCCCCDDDDDDDDDDEEEEFFHLKHHGGGGHHMJHGGGGGGHHHIKRR                                           UQ L HFEDDDDCCCCCCCCCCCCCCBB
    AABCCCCCCCCDDDDDDDDDDDEEEEEEFFFHKQMRKNJIJLVS JJKIIIIIIJLR                                               YNHFEDDDDDCCCCCCCCCCCCCBB
    AABCCCCCDDDDDDDDDDDDEEEEEEEFFGGHIJKOU  O O   PR LLJJJKL                                                OIHFFEDDDDDCCCCCCCCCCCCCCB
    AACCCDDDDDDDDDDDDDEEEEEEEEEFGGGHIJMR              RMLMN                                                 NTFEEDDDDDDCCCCCCCCCCCCCB
    AACCDDDDDDDDDDDDEEEEEEEEEFGGGHHKONSZ                QPR                                                NJGFEEDDDDDDCCCCCCCCCCCCCC
    ABCDDDDDDDDDDDEEEEEFFFFFGIPJIIJKMQ                   VX                                                 HFFEEDDDDDDCCCCCCCCCCCCCC
    ACDDDDDDDDDDEFFFFFFFGGGGHIKZOOPPS                                                                      HGFEEEDDDDDDCCCCCCCCCCCCCC
    ADEEEEFFFGHIGGGGGGHHHHIJJLNY                                                                        TJHGFFEEEDDDDDDDCCCCCCCCCCCCC
    A                                                                                                 PLJHGGFFEEEDDDDDDDCCCCCCCCCCCCC
    ADEEEEFFFGHIGGGGGGHHHHIJJLNY                                                                        TJHGFFEEEDDDDDDDCCCCCCCCCCCCC
    ACDDDDDDDDDDEFFFFFFFGGGGHIKZOOPPS                                                                      HGFEEEDDDDDDCCCCCCCCCCCCCC
    ABCDDDDDDDDDDDEEEEEFFFFFGIPJIIJKMQ                   VX                                                 HFFEEDDDDDDCCCCCCCCCCCCCC
    AACCDDDDDDDDDDDDEEEEEEEEEFGGGHHKONSZ                QPR                                                NJGFEEDDDDDDCCCCCCCCCCCCCC
    AACCCDDDDDDDDDDDDDEEEEEEEEEFGGGHIJMR              RMLMN                                                 NTFEEDDDDDDCCCCCCCCCCCCCB
    AABCCCCCDDDDDDDDDDDDEEEEEEEFFGGHIJKOU  O O   PR LLJJJKL                                                OIHFFEDDDDDCCCCCCCCCCCCCCB
    AABCCCCCCCCDDDDDDDDDDDEEEEEEFFFHKQMRKNJIJLVS JJKIIIIIIJLR                                               YNHFEDDDDDCCCCCCCCCCCCCBB
    AAABCCCCCCCCCCCDDDDDDDDDDEEEEFFHLKHHGGGGHHMJHGGGGGGHHHIKRR                                           UQ L HFEDDDDCCCCCCCCCCCCCCBB
    AAABCCCCCCCCCCCCCCCCCDDDDDDDEEFJIHFFFFFFFFFFFFFFGGGGGGHIJN                                            JHHGFEEDDDDCCCCCCCCCCCCCBBB
    AAAABCCCCCCCCCCCCCCCCCCCCCCDDDDEEEEEEEEEEEEEEEEFFFFFFGGHYV RQU                                     QMJHGGFEEEDDDCCCCCCCCCCCCCBBBB
    AAAABBCCCCCCCCCCCCCCCCCCCCCCCCCDDDDEEEEEEEEEEEEEEEFFFFFFGHIJKLOT                                     [JGFFEEEDDCCCCCCCCCCCCCBBBBB
    AAAAABBCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDEEEEEEEEEEEEFFFFFGHHIN                                 Q     UMWGEEEDDDCCCCCCCCCCCCBBBBBB
    AAAAABBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDEEEEEEEEEFFFFGH O    TN S                       NKJKR LLQMNHEEDDDCCCCCCCCCCCCBBBBBBB
    AAAAAABBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDEEEEEEFFGHK   MKJIJO  N R  X      YUSR PLV LHHHGGHIOJGFEDDDCCCCCCCCCCCCBBBBBBBB
    AAAAAAABBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDEEEEEFGGHIIHHHHHIIIJKMR        VMKJIHHHGFFFFFFGSGEDDDDCCCCCCCCCCCCBBBBBBBBB
    AAAAAAABBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEFFFFFFGGGGHIKP           KHHGGFFFFEEEEEEDDDDDCCCCCCCCCCCBBBBBBBBBBB
    AAAAAAAABBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEFFFFFGGHJLZ         UKHGFFEEEEEEEEDDDDDCCCCCCCCCCCCBBBBBBBBBBBB
    AAAAAAAAABBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEEFFGQPUVOTY   ZQL[MHFEEEEEEEDDDDDDDCCCCCCCCCCCBBBBBBBBBBBBBB
    AAAAAAAAAABBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDDEEEEEEFFGHIJKS  X KHHGFEEEEEDDDDDDDDDCCCCCCCCCCBBBBBBBBBBBBBBBB
    AAAAAAAAAAABBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEEEFGGHHIKPPKIHGFFEEEDDDDDDDDDCCCCCCCCCCBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAABBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDDDEEEEEFFGHIMTKLZOGFEEDDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAAABBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDDDEEEEFFFI KHGGGHGEDDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBBBB
    AAAAAAAAAAAAAAABBBBBBBBBBBBBCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCCDDDDDDDDDDEEEFGIIGFFEEEDDDDDDDDCCCCCCCCCBBBBBBBBBBBBBBBBBBBBBBBBBB

    real    992m9.836s
    user    991m45.631s
    sys 0m3.060s

It took 992 minutes (16hs 32 min). Not very fast...

Now with the C interpreter:

    $ time ./brainfuck ../samples/mandelbrot.bf
    [...]
    real    1m50.316s
    user    1m50.251s
    sys 0m0.024s

1m50s! Way faster than running with the Python interpreter.

3rd try: the translated to C version without optimizations:

    $ runhaskell bf2c.hs < ../samples/mandelbrot.bf > mandelbrot.c
    $ cc mandelbrot.c -o mandelbrot
    $ time ./mandelbrot
    [...]
    real    0m18.084s
    user    0m18.033s
    sys 0m0.032s

An improvement, but not as impressive as in the first case.

Finally the same C version, but compiled with optimizations:

    $ cc -O3 mandelbrot.c -o mandelbrot
    $ time ./mandelbrot
    [...]
    real    0m1.111s
    user    0m1.092s
    sys 0m0.004s

Now THAT's fast.

## Improvements in the Python version

The original Python interpreter is excessively complex, but it can be easily improved to run faster in several ways:

 * Avoiding methods lookup
 * Pre-computing jumps between brackets
 * Avoiding looping over non-operands
 * Avoiding array lookups

There's a separate version, brainfuck-simple.py that contains those improvements. Another version, brainfuck-rpython.py, is the same thing but slightly modified so it can be translated using RPython. The RPython version was generated using: 

```
$ cd <pypy-source>
$ python rpython/translator/goal/translate.py ~/Projects/github/brainfuck/python/brainfuck-rpython.py
```

## Comparison table

This is the full comparison between all versions:

| | Sierpinski | Mandelbrot | Primes up to 100 | 
| ---: | :---: | :---: | :---: |
| Non-optimized python version (CPython) | 0m5.387s | 991m45.631s | 19m34.163s |
| Non-optimized python version (PyPy) | 0m0.470s | 24m59.928s | 0m28.210s |
| Optimized python version (CPython) | 0m0.125s | 67m39.287s | 1m16.431s |
| Optimized python version (PyPy) | 0m0.246s | 1m35.345s | 0m2.144s |
| Improved python version (RPython) | 0m0.003s | 0m29.796s | 0m0.486s |
| Assembler | 0m0.015s | 1m7.288s | 0m1.501s |
| C Interpreter (-O0) | 0m0.014s | 2m7.036s | 0m2.012s |
| C Interpreter (-O1) | 0m0.009s | 1m7.504s | 0m1.005s |
| Translated to C (-O0) | 0m0.002s | 0m19.674s | 0m0.243s |
| Translated to C (-O1) | 0m0.001s | 0m1.360s | 0m0.012s |

Notice the impressive difference between CPython and PyPy. In both cases, running the same code is 40 times slower in CPython. This means you can have a really big gain for "free" (almost), by just using the PyPy interpreter. Translating to RPython is not free of course, and the gain is not as big.

In the other cases, the performance differences are totally expected (C interpreter compiled with optimisations has an equivalent performance to the assembler interpreter, the translated to C version is almost two orders of magnitude faster than the interpreted version, etc.).

