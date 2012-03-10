ALL = brainfuck

all: $(ALL)

brainfuck: brainfuck.c
	$(CC) $< -o $@
