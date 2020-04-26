ALL = brainfuck translate

all: $(ALL)

test: tests
	./$<

brainfuck: brainfuck.rs bf.rs
translate: translate.rs

%: %.rs
	rustc -O $<

%: %.c
	$(CC) $< -o $@

clean:
	rm -rf $(ALL) tests

%.rs: %.bf
	./translate < $< | rustfmt > $(notdir $@)

%.c: %.bf
	./translate c < $< | indent > $(notdir $@)
