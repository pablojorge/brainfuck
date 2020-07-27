ALL = brainfuck brainfuck-oop

# CXX = g++-10
CXX = c++
CXXFLAGS = -std=c++14 -g #-O3

all: $(ALL)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

dump:
	objdump -dSC ./brainfuck-oop

debug: brainfuck-oop
	lldb -s lldb-commands.txt ./brainfuck-oop -- test.bf

clean:
	rm -rf $(ALL) *.o
