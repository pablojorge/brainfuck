ALL = brainfuck

all: $(ALL)

%: %.cpp
	$(CXX) -std=c++17 -g -O3 $< -o $@

dis:
	objdump -dSC ./brainfuck

clean:
	rm -rf $(ALL) *.o
