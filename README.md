# Brainfuck Experiments

This project contains several interpreters for the [brainfuck language](http://www.muppetlabs.com/~breadbox/bf/). Currently, this is what's available:

 * [python/brainfuck.py](brainfuck/blob/master/python/brainfuck.py): Interpreter in Python
 * [c/brainfuck.c](brainfuck/blob/master/c/brainfuck.c): Interpreter in C
 * [haskell/brainfuck.hs](brainfuck/blob/master/haskell/brainfuck.hs): Interpreter in Haskell
 * [haskell/bf2c.hs](brainfuck/blob/master/haskell/bf2c.hs): Translator from brainfuck to C in Haskell
 * [asm/brainfuck.s](brainfuck/blob/master/asm/brainfuck.s): Interpreter in assembler for x86_64
 
Is also includes a series of sample programs:

 * [samples/hello.bf](brainfuck/blob/master/samples/hello.bf): Simple hello world! program
 * [samples/primes.bf](brainfuck/blob/master/samples/primes.bf): Prime number generator. It prompts a number and generates all the primes from 1 up to that number.
 * [samples/rot13.bf](brainfuck/blob/master/samples/rot13.bf): Applies ROT13 to its input
 * [samples/fibonacci.bf](brainfuck/blob/master/samples/fibonacci.bf): Fibonacci number generator.
 * [samples/mandelbrot.bf](brainfuck/blob/master/samples/mandelbrot.bf): Mandelbrot set generator (taken from [http://esoteric.sange.fi/brainfuck/bf-source/prog/mandelbrot.b](http://esoteric.sange.fi/brainfuck/bf-source/prog/mandelbrot.b))
 * [samples/sierpinski.bf](brainfuck/blob/master/samples/sierpinski.bf): Sierpinsky Triangle generator (taken from the [spanish Wikipedia article of Brainfuck](http://es.wikipedia.org/wiki/Brainfuck))
 
And very simple programs I wrote myself:

 * [programs/cat.bf](brainfuck/blob/master/programs/cat.bf): Emulates the "cat" program. It's just "+[,.]"
 * [programs/tolower.bf](brainfuck/blob/master/programs/tolower.bf): Prints the lower case equivalent of its input, but it's no so smart since it doesn't check for the original case or whether it's a letter or not.
 
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

## Speed differences between each version

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

