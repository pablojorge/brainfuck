ALL = brainfuck

all: $(ALL)

brainfuck: brainfuck.c
	$(CC) $< -o $@

%.c: %.bf
	runhaskell bf2c.hs < $< | indent > $@

%: %.c
	$(CC) $< -o $@

clean:
	rm -rf $(ALL) *.hi *.o

