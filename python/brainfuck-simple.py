import sys
import optparse

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
            sys.stdout.write(chr(buffer[ptr]))
            sys.stdout.flush()
        elif opcode == ",": 
            buffer[ptr] = ord(sys.stdin.read(1))
        elif opcode == "[":
            if buffer[ptr] == 0: pc = jump_map[pc]
        elif opcode == "]":
            if buffer[ptr] != 0: pc = jump_map[pc]
        pc += 1

if __name__ == "__main__":
    parser = optparse.OptionParser()
    parser.add_option("-v", "--verbose", dest="verbose",
                      action="store_true", default=False,
                      help="Verbosity ON")

    options, args = parser.parse_args()

    if args:
        with open(args[0], "r") as input_file:
            contents = input_file.read()
    else:
        contents = sys.stdin.read()

    program = filter(lambda c: c in "<>-+[],.", contents)
    run(program)
