all:
	cargo build

%: %.rs
	rustc -C opt-level=3 $< -o $@

%: %.c
	$(CC) -O1 $< -o $@

clean:
	rm -rf tests $(shell cat .gitignore)