ALL = main bf2rs

all: $(ALL)

test: tests
	./$<

main: main.rs brainfuck.rs
bf2rs: bf2rs.rs

%: %.rs
	rustc -O $<

%: %.c
	$(CC) $< -o $@

clean:
	rm -rf $(ALL) tests

%.rs: %.bf
	./bf2rs < $< | rustfmt > $(notdir $@)

%.c: %.bf
	./bf2rs c < $< | indent > $(notdir $@)
