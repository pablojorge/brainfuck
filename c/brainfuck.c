#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

enum direction {
    FWD,
    BWD
};

char* find_match(char *src, char match, enum direction dir) {
    char *aux = src;
    int count = 1;

    while (count) {
        dir == FWD ? ++aux : --aux;
        if (*aux == *src) ++count;
        if (*aux == match) --count;
    }

    return aux;
}

void prepare_jumps(char *program, ssize_t prg_size, char *jump_map[]) {
    char *aux = program;

    while (aux < (program + prg_size)) {
        switch(*aux) {
            case '[':
                jump_map[aux-program] = find_match(aux, ']', FWD);
                break;
            case ']':
                jump_map[aux-program] = find_match(aux, '[', BWD);
                break;
        }
        ++aux;
    }
}

ssize_t read_prg(char *buf, ssize_t buf_size, int fd) {
    char *aux_ptr = buf;
    ssize_t prg_size = 0,
            aux_size = buf_size;

    ssize_t ret = read(fd, aux_ptr, aux_size);

    while (ret && (prg_size < buf_size)) {
        aux_size -= ret;
        prg_size += ret;
        aux_ptr += ret;
        ret = read(fd, aux_ptr, aux_size);
    }

    return prg_size;
}

int main(int argc, char *argv[])
{
    char program[50000],
         memory[30000];
    char *jump_map[50000];
    char *ip = program,
         *ptr = memory;
    int fd = 0;
    ssize_t prg_size = 0;

    if (argc < 2) {
        fd = STDIN_FILENO;
    } else {
        fd = open(argv[1], O_RDONLY);

        if(fd < 0){
            fprintf(stderr, "Error opening %s!\n", argv[1]);
            perror("open");
            exit(1);
        }
    }

    prg_size = read_prg(program, sizeof(program), fd);

    prepare_jumps(program, prg_size, jump_map);

    while (ip < (program + prg_size)) {
        switch(*ip) {
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
                    ip = jump_map[ip-program];
                }
                break;
            case ']':
                if (*ptr) {
                    ip = jump_map[ip-program];
                }
                break;
        }
        ++ip;
    }

    return 0;
}
