import sys

class Buffer:
    def __init__(self, size):
        self.array = [0] * size
        self.ptr = 0

    def move(self, n):
        self.ptr += n
        assert(0 <= self.ptr < len(self.array))

    def increment(self):
        assert(0 <= self.ptr < len(self.array))
        self.array[self.ptr]+=1

    def decrement(self):
        assert(0 <= self.ptr < len(self.array))
        self.array[self.ptr]-=1

    def current(self):
        assert(0 <= self.ptr < len(self.array))
        return self.array[self.ptr]

    def store(self, val):
        assert(0 <= self.ptr < len(self.array))
        self.array[self.ptr] = val

    def __str__(self):
        return "ptr: %d value: %d" % (self.ptr, self.current())

class Program:
    def __init__(self, program):
        self.program = program
        self.pos = 0

    def advance(self, n=1):
        self.pos += n
        assert(0 <= self.pos <= len(self.program)) # tolerate off-by-one AT END

    def current(self):
        assert(0 <= self.pos < len(self.program))
        return self.program[self.pos]

    def eof(self):
        return self.pos == len(self.program)

    def __str__(self):
        return "pos: %d op: '%s'" % (self.pos, self.current())

class EOFException(Exception):
    def __init__(self):
        Exception.__init__(self, "EOF")

class StdinStream:
    def __init__(self):
        pass

    def put(self, val):
        raise Exception("invalid")

    def get(self):
        data = sys.stdin.read(1)
        if not data:
            raise EOFException()
        return data[0]

class StdoutStream:
    def __init__(self):
        pass

    def put(self, val):
        sys.stdout.write(val)

    def get(self):
        raise Exception("invalid")

    def dump(self):
        sys.stdout.flush()

class BufStream:
    def __init__(self, buffer):
        self.buffer = buffer
        self.pos = 0

    def put(self, val):
        self.buffer.append(val)

    def get(self):
        if self.pos == (self.buffer):
            raise EOFException()
        ret = self.buffer[self.pos]
        self.pos+=1
        return ret

    def __str__(self):
        return "".join(self.buffer)

    def dump(self):
        sys.stdout.write(str(self))

class Interpreter:
    INC_PTR       = ">"
    DEC_PTR       = "<"
    INC_BYTE      = "+"
    DEC_BYTE      = "-"
    OUTPUT_BYTE   = "."
    INPUT_BYTE    = ","
    JUMP_FORWARD  = "["
    JUMP_BACKWARD = "]"

    def __init__(self, program, buffer, instream, outstream):
        self.program = program
        self.buffer = buffer
        self.instream = instream
        self.outstream = outstream

    def __handle_inc_ptr(self):
        """
        increment the data pointer (to point to the next cell to the right).
        """
        self.buffer.move(+1)

    def __handle_dec_ptr(self):
        """
        decrement the data pointer (to point to the next cell to the left).
        """
        self.buffer.move(-1)

    def __handle_inc_byte(self):
        """increment (increase by one) the byte at the data pointer."""
        self.buffer.increment()

    def __handle_dec_byte(self):
        """decrement (decrease by one) the byte at the data pointer."""
        self.buffer.decrement()

    def __handle_output_byte(self):
        """
        output the byte at the data pointer as an ASCII encoded character.
        """
        self.outstream.put(chr(self.buffer.current()))

    def __handle_input_byte(self):
        """
        accept one byte of input, storing its value in the byte at the data 
        pointer.
        """
        self.buffer.store(ord(self.instream.get()))

    def __handle_jump_forward(self):
        """
        if the byte at the data pointer is zero, then instead of moving the 
        instruction pointer forward to the next command, jump it forward to 
        the command after the matching ] command.
        """
        if self.buffer.current() == 0:
            count = 1
            while count:
                self.__dump_state("__handle_jump_forward: (count: %d)" % count)
                self.program.advance()
                if self.program.current() == self.JUMP_FORWARD:
                    count += 1
                elif self.program.current() == self.JUMP_BACKWARD:
                    count -= 1

    def __handle_jump_backward(self):
        """
        if the byte at the data pointer is nonzero, then instead of moving 
        the instruction pointer forward to the next command, jump it back 
        to the command after the matching [ command.
        """
        # (Alternatively, the ] command may instead be translated as an 
        # unconditional jump to the corresponding [ command, or vice versa; 
        # programs will behave the same but will run more slowly, due to 
        # unnecessary double searching.)
        if self.buffer.current() != 0:
            count = 1
            while count:
                self.__dump_state("__handle_jump_backward: (count: %d)" % count)
                self.program.advance(-1)
                if self.program.current() == self.JUMP_BACKWARD:
                    count += 1
                elif self.program.current() == self.JUMP_FORWARD:
                    count -= 1

    def __dump_state(self, msg):
        pass
        #print msg, self.program, self.buffer

    def execute(self):
        op_handler = {
            self.INC_PTR       : self.__handle_inc_ptr,
            self.DEC_PTR       : self.__handle_dec_ptr,
            self.INC_BYTE      : self.__handle_inc_byte,
            self.DEC_BYTE      : self.__handle_dec_byte,
            self.OUTPUT_BYTE   : self.__handle_output_byte,
            self.INPUT_BYTE    : self.__handle_input_byte,
            self.JUMP_FORWARD  : self.__handle_jump_forward,
            self.JUMP_BACKWARD : self.__handle_jump_backward
        }

        while not self.program.eof():
            self.__dump_state("execute:")
            handler = op_handler.get(self.program.current())
            if handler:
                handler()
            self.program.advance()

if __name__ == "__main__":
    with open(sys.argv[1], "r") as input_file:
        contents = input_file.read()

    # Define program
    program = Program(contents)
    #program = Program("[[[]]]")

    # Memory buffer
    buffer = Buffer(30000)

    # Input stream
    #instream = BufStream("asdf")
    instream = StdinStream() 

    # Output stream
    #outstream = BufStream([])
    outstream = StdoutStream()

    try:
        interpreter = Interpreter(program, 
                                  buffer, 
                                  instream, 
                                  outstream)
        interpreter.execute()
    except EOFException:
        pass
    except KeyboardInterrupt:
        pass
    except Exception, e:
        print >> sys.stderr, "ERROR:", e

    outstream.dump()
