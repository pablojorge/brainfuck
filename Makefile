ALL = brainfuck

all: $(ALL)

brainfuck: brainfuck.c
	$(CC) $< -o $@

%.c: %.bf
	runhaskell bf2c.hs < $< | indent > $@

%: %.c
	$(CC) $< -o $@

%: %.s
	as -arch x86_64 $< -o $(subst .s,.o,$<)
	ld -e _main -arch x86_64 -lc $(subst .s,.o,$<) -o $@ 

clean:
	rm -rf $(ALL) *.hi *.o

