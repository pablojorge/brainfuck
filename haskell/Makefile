ALL = brainfuck

all: $(ALL)

%: %.c
	$(CC) $< -o $@ 

%.c: %.bf
	runhaskell bf2c.hs < $< | indent > $(notdir $@)

clean:
	rm -rf $(ALL) *.o

