import os

def precompute_jumps(program):
    stack = []
    ret = {}

    pc = 0

    while not pc == len(program):
        opcode = program[pc]
        if opcode == "[":
            stack.append(pc)
        elif opcode == "]":
            target = stack.pop()
            ret[target] = pc
            ret[pc] = target 
        pc += 1

    return ret

def run(program):
    buffer = [0]
    jump_map = precompute_jumps(program)

    ptr = 0
    pc = 0

    while not pc == len(program):
        opcode = program[pc]
        if opcode == ">": 
            ptr += 1
            if ptr == len(buffer): buffer.append(0)
        elif opcode == "<": ptr -= 1
        elif opcode == "+": buffer[ptr] += 1
        elif opcode == "-": buffer[ptr] -= 1
        elif opcode == ".": 
            os.write(1, chr(buffer[ptr]))
        elif opcode == ",": 
            buffer[ptr] = ord(os.read(0, 1)[0])
        elif opcode == "[":
            if buffer[ptr] == 0: pc = jump_map[pc]
        elif opcode == "]":
            if buffer[ptr] != 0: pc = jump_map[pc]
        pc += 1

def main(argv):
    if not len(argv) > 1:
        print "Missing input filename"
        return 1

    fd = os.open(argv[1], os.O_RDONLY, 0777)
    size = os.stat(argv[1]).st_size
    contents = os.read(fd, size)
    os.close(fd)

    program = []
    for c in contents:
        if c in "<>-+[],.":
            program.append(c)

    run(program)

    return 0

def target(driver, args):
    return main, None

if __name__ == "__main__":
    import sys
    main(sys.argv)
