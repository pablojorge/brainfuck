ALL = brainfuck

all: $(ALL)

brainfuck: brainfuck.o

%.o: %.s
	as -arch x86_64 $< -o $@

%: %.o
	ld -e _main -arch x86_64 -lc $< -o $@ 

debug: brainfuck
# 	gdb -x gdb-commands.txt --args ./brainfuck ../programs/cat.bf
	lldb -s lldb-commands.txt ./brainfuck -- ../programs/cat.bf

dump:
	objdump -dS ./brainfuck

clean:
	rm -rf $(ALL) *.o

