ALL = brainfuck-adt brainfuck-jit brainfuck-oop

# CXX = g++-10
CXX = c++
CXXFLAGS = -std=c++14 -g -O3

all: $(ALL)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

%.o: %.s
	as -arch x86_64 $< -o $@

dump: brainfuck.o
	objdump -dS ./brainfuck.o

debug: brainfuck-jit
	lldb -s lldb-commands.txt ./brainfuck-jit -- test.bf

clean:
	rm -rf $(ALL) *.o
