ALL = brainfuck

all: $(ALL)

%: %.c
	$(CC) -O3 $< -o $@

clean:
	rm -rf $(ALL) *.o
