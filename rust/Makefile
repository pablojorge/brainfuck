all:
	cargo build

%: %.c
	$(CC) -O1 $< -o $@

test:
	cargo test

clean:
	rm -rf target
