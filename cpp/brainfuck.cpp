#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

auto prepare_jumps(std::vector<char> &program) {
    std::array<int, 50000> jump_map;
    ssize_t prg_size = program.size();
    int aux = 0;

    std::array<int, 50000> stack;
    int stack_ptr = 0;

    while (aux < prg_size) {
        switch(program[aux]) {
            case '[':
                stack[stack_ptr++] = aux;
                break;
            case ']':
                int target = stack[--stack_ptr];
                jump_map[aux] = target;
                jump_map[target] = aux;
                break;
        }
        ++aux;
    }

    return std::move(jump_map);
}

int main(int argc, char *argv[]) {
    std::array<char, 30000> memory;
    std::ifstream ifs(argv[1]);

    if (!ifs.good()) {
        std::cerr << "Invalid filename!" << std::endl;
        return 1;
    }

    std::string input((std::istreambuf_iterator<char>(ifs)),
                      (std::istreambuf_iterator<char>()));

    std::vector<char> program;
    std::string bf_operators("+-<>[],.");

    std::copy_if(
        input.begin(), input.end(),
        std::back_inserter(program),
        [bf_operators](char c){
            return bf_operators.find(c) != std::string::npos;
        }
    );

    auto jump_map = prepare_jumps(program);

    int ip = 0;

    auto ptr = memory.begin();

    ssize_t prg_size = program.size();

    while (ip < prg_size) {
        switch(program[ip]) {
            case '>': ++ptr; break;
            case '<': --ptr; break;
            case '+': ++(*ptr); break;
            case '-': --(*ptr); break;
            case '.': putchar(*ptr); 
                      fflush(stdout); 
                      break;
            case ',': *ptr = getchar(); 
                      if (*ptr == EOF) exit(0);
                      break;
            case '[':
                if (!*ptr) {
                    ip = jump_map[ip];
                }
                break;
            case ']':
                if (*ptr) {
                    ip = jump_map[ip];
                }
                break;
        }
        ++ip;
    }

    return 0;
}
