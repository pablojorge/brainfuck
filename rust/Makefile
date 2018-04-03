ALL = main

all: $(ALL)

test: tests
	./$<

main: main.rs brainfuck.rs

%: %.rs
	rustc $<

clean:
	rm -rf $(ALL) tests
