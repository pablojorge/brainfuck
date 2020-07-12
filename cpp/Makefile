ALL = brainfuck brainfuck-oop

# CXX = g++-10
CXX = c++
CXXFLAGS = -std=c++14 -g -O3

all: $(ALL)

%: %.cpp
	$(CXX) $(CXXFLAGS) $< -o $@

dis:
	objdump -dSC ./brainfuck

clean:
	rm -rf $(ALL) *.o
