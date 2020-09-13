all:
	cargo build

%: %.c
	$(CC) -O1 $< -o $@

test:
	cargo test

clean:
	rm -rf target

%.o: %.s
	as -arch x86_64 $< -o $@

dump: brainfuck.o
	objdump -dS ./brainfuck.o

debug: all
	lldb -s lldb-commands.txt target/debug/jit -- ../programs/cat.bf
