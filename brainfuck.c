#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
    char program[50000],
         memory[30000];
    char *ip = program,
         *ptr = memory,
         *aux_ptr = NULL;
    int fd = 0;
    ssize_t prg_size = 0,
            aux_size = 0,
            ret = 0;
    int count = 0;

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

    aux_size = sizeof(program);
    aux_ptr = program;
    ret = read(fd, aux_ptr, aux_size);

    while (ret && (prg_size < sizeof(program))) {
        aux_size -= ret;
        prg_size += ret;
        aux_ptr += ret;
        ret = read(fd, aux_ptr, aux_size);
    }

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
                    count = 1;
                    while (count) {
                        ++ip;
                        if (*ip == '[') ++count;
                        if (*ip == ']') --count;
                    }
                }
                break;
            case ']':
                if (*ptr) {
                    count = 1;
                    while (count) {
                        --ip;
                        if (*ip == ']') ++count;
                        if (*ip == '[') --count;
                    }
                }
                break;
        }
        ++ip;
    }

    return 0;
}
