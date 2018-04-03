ALL = brainfuck

all: $(ALL)

test: tests
	./$<

%: %.rs
	rustc $<

clean:
	rm -rf $(ALL) *.o
