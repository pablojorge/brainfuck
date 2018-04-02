ALL = brainfuck

all: $(ALL)

%: %.rs
	rustc $<

clean:
	rm -rf $(ALL) *.o
