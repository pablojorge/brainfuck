ALL = brainfuck

all: $(ALL)

%.o: %.asm
	as -arch x86_64 $< -o $@

%: %.o
	ld -e _main -arch x86_64 -lc $< -o $@ 

debug: brainfuck
	gdb -x gdb-commands.txt --args ./brainfuck ../programs/cat.bf

clean:
	rm -rf $(ALL) *.o

