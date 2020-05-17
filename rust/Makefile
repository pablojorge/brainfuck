ALL = translate

all: $(ALL)

test: tests
	./$<

brainfuck: src/main.rs src/bf.rs
translate: src/translate.rs
tests: src/tests.rs

%: src/%.rs
	rustc -O $<

%: src/%.c
	$(CC) $< -o $@

clean:
	rm -rf $(ALL) tests

%.rs: %.bf
	./translate < $< | rustfmt > src/$(notdir $@)

%.c: %.bf
	./translate c < $< | indent > src/$(notdir $@)
