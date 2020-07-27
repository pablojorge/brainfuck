ALL = brainfuck

all: $(ALL)

%: %.c
	$(CC) -g -O3 $< -o $@

dump:
	objdump -dS ./brainfuck

clean:
	rm -rf $(ALL) *.o *.dSYM
