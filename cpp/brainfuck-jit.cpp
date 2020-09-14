#include <fstream>
#include <iostream>
#include <vector>

#include <sys/mman.h>
#include <cstring>

#include "brainfuck.h"

// Wraps an mmap()ed area, which starts with read/write permissions,
// but that can be later turned into read/exec before execution.
// This is good practice since the pages are never writable AND executable
// _at the same_time_
// (See https://eli.thegreenplace.net/2013/11/05/how-to-jit-an-introduction)
class ExecutableBuffer
{
private:
    uint8_t *buf_,
            *ptr_;
    size_t size_;

public:
    ExecutableBuffer(size_t size) {
        size_ = size;
        buf_ = (uint8_t*) mmap(
            0,
            size_,
            PROT_READ | PROT_WRITE,
            MAP_PRIVATE | MAP_ANONYMOUS,
            -1,
            0
        );
        if (buf_ == MAP_FAILED) {
            perror("mmap");
            exit(1);
        }
        ptr_ = buf_;
        memset(ptr_, 0x00, size);
    }

    ~ExecutableBuffer() {
        if(munmap(buf_, size_) == -1) {
            perror("munmap");
            exit(1);
        }
    }

    void make_executable() {
        if (mprotect(buf_, size_, PROT_READ | PROT_EXEC) == -1) {
            perror("mprotect");
            exit(1);
        }
    }

    uint8_t* get_base () const { return buf_; }
    uint8_t* get_ptr () const { return ptr_; }
    void set_ptr (uint8_t* ptr) { ptr_ = ptr; }

    void writeb(uint8_t byte) {
        // write a single byte
        (*ptr_++) = byte;
    }

    void writel(uint32_t value) {
        // write a 4-bytes word, keeping endianness
        writes((uint8_t*)&value, 4);
    }

    void writes(const uint8_t* bytes, uint32_t size) {
        // write an arbitrary-length series of bytes
        memcpy(ptr_, bytes, size);
        ptr_ += size;
    }
};

class JITProgram
{
private:
    ExecutableBuffer &buf_;

public:
    JITProgram(ExecutableBuffer &buf)
      : buf_(buf) {}

    ExecutableBuffer& buffer() {return buf_;}

    void start() {
        // 0000000000000070 break:
        //       70: cc                            int3
        // buf_.writeb(0xcc);
    }

    void finish() {
        // 0000000000000071 finish:
        //       71: c3                            retq
        buf_.writeb(0xc3);
    }

    void run() {
        uint32_t memory[30000];
        memset(memory, 0x00, sizeof(memory));

        // Set the buffer as executable before attempting to jump
        // into it:
        buf_.make_executable();

        // Capture the buffer base ptr:
        uint8_t* _base = buf_.get_base();

        // Inject the address of the working memory in the rdi reg:
        asm("movq %0, %%rdi\n":
            : "r"(&memory)
            : "%rdi");

        // Cast the base as a func pointer and jump to it:
        ((void (*)()) _base)();
    }
};

class JITCompiler : public ExpressionVisitor
{
private:
    JITProgram &program_;
    ExecutableBuffer &buffer_;

public:
    JITCompiler(JITProgram &program)
      : program_(program),
        buffer_(program_.buffer()) {}

    virtual void visit(const Increment& inc) {
        // 0000000000000000 increment:
        //        0: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //        7: 01 07                         addl    %eax, (%rdi)
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(inc.offset());
        buffer_.writes((uint8_t*)"\x01\x07", 2);        
    }

    virtual void visit(const Decrement& dec) {
        // 0000000000000009 decrement:
        //        9: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       10: 29 07                         subl    %eax, (%rdi)
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(dec.offset());
        buffer_.writes((uint8_t*)"\x29\x07", 2);
    }

    virtual void visit(const Forward& fwd) {
        // 0000000000000012 forward:
        //       12: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       19: 48 01 c7                      addq    %rax, %rdi
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(fwd.offset()*4);
        buffer_.writes((uint8_t*)"\x48\x01\xc7", 3);
    }

    virtual void visit(const Backward& bwd) {
        // 000000000000001c backward:
        //       1c: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       23: 48 29 c7                      subq    %rax, %rdi
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(bwd.offset()*4);
        buffer_.writes((uint8_t*)"\x48\x29\xc7", 3);
    }

    virtual void visit(const Input&) {
        // 0000000000000026 read:
        //       26: 57                            pushq   %rdi
        //       27: 48 c7 c0 03 00 00 02          movq    $33554435, %rax
        //       2e: 48 89 fe                      movq    %rdi, %rsi
        //       31: 48 c7 c7 00 00 00 00          movq    $0, %rdi
        //       38: 48 c7 c2 01 00 00 00          movq    $1, %rdx
        //       3f: 0f 05                         syscall
        //       41: 5f                            popq    %rdi        

        buffer_.writeb(0x57);
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(0x02000003);
        buffer_.writes((uint8_t*)"\x48\x89\xfe", 3);
        buffer_.writes((uint8_t*)"\x48\xc7\xc7\x00\x00\x00\x00", 7);
        buffer_.writes((uint8_t*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        buffer_.writes((uint8_t*)"\x0f\x05", 2);
        buffer_.writeb(0x5f);
    }

    virtual void visit(const Output&) {
        // 0000000000000042 write:
        //       42: 57                            pushq   %rdi
        //       43: 48 c7 c0 04 00 00 02          movq    $33554436, %rax
        //       4a: 48 89 fe                      movq    %rdi, %rsi
        //       4d: 48 c7 c7 01 00 00 00          movq    $1, %rdi
        //       54: 48 c7 c2 01 00 00 00          movq    $1, %rdx
        //       5b: 0f 05                         syscall
        //       5d: 5f                            popq    %rdi

        buffer_.writeb(0x57);
        buffer_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        buffer_.writel(0x02000004);
        buffer_.writes((uint8_t*)"\x48\x89\xfe", 3);
        buffer_.writes((uint8_t*)"\x48\xc7\xc7\x01\x00\x00\x00", 7);
        buffer_.writes((uint8_t*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        buffer_.writes((uint8_t*)"\x0f\x05", 2);
        buffer_.writeb(0x5f);
    }

    virtual void visit(const Loop& loop) {
        // 000000000000005e loop_start:
        //       5e: 83 3f 00                      cmpl    $0, (%rdi)
        //       61: 0f 84 00 00 00 00             je  0
        buffer_.writes((uint8_t*)"\x83\x3f\x00", 3);
        buffer_.writes((uint8_t*)"\x0f\x84", 2);
        buffer_.writel(0); // reserve 4 bytes

        // Save current position:
        uint8_t *after_loop_start = buffer_.get_ptr();

        // Recurse into subexpressions:
        for(const auto &child: loop.children()) {
            child->accept(*this);
        }

        // 0000000000000067 loop_end:
        //       67: 83 3f 00                      cmpl    $0, (%rdi)
        //       6a: 0f 85 00 00 00 00             jne 0
        buffer_.writes((uint8_t*)"\x83\x3f\x00", 3);
        buffer_.writes((uint8_t*)"\x0f\x85", 2);
        // Calculate how much to jump back (consider the 4 bytes
        // of the operand itself):
        uint32_t jump_back = after_loop_start - buffer_.get_ptr() - 4;
        // Append the distance to the jump:
        buffer_.writel(jump_back);

        // Now calculate how much to jump forward in case we want
        // to skip the loop, to fill the pending jump:
        uint8_t *after_loop_end = buffer_.get_ptr();
        uint32_t jump_fwd = after_loop_end - after_loop_start;

        // Go back to the original position (-4), fill the pending
        // jump distance and get back to the the current pos:
        buffer_.set_ptr(after_loop_start - 4);
        buffer_.writel(jump_fwd);
        buffer_.set_ptr(after_loop_end);
    }

    void compile(const ExpressionVector& expressions) {
        program_.start();

        for(const auto &expression: expressions) {
            expression->accept(*this);
        }

        program_.finish();
    }
};

int main(int argc, char *argv[]) {
    std::ifstream ifs(argv[1]);

    if (!ifs) {
        std::cerr << "Invalid filename!" << std::endl;
        return 1;
    }

    std::vector<char> program;

    std::copy(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()),
        std::back_inserter(program)
    );

    try {
        auto expressions = Parser().parse(program);

        ExecutableBuffer buffer(1000000);
        JITProgram jit_program(buffer);
        JITCompiler compiler(jit_program);

        compiler.compile(expressions);

        jit_program.run();

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
