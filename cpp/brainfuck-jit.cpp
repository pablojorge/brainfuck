#include <fstream>
#include <iostream>
#include <vector>

#include <sys/mman.h>
#include <cstring>

#include "brainfuck.h"


// Sets a RX permission on the given memory, which must be page-aligned. Returns
// 0 on success. On failure, prints out the error and returns -1.
int make_memory_executable(void* m, size_t size) {
  if (mprotect(m, size, PROT_READ | PROT_EXEC) == -1) {
    perror("mprotect");
    return -1;
  }
  return 0;
}

class JITProgram
{
private:
    uint8_t *program_, *ptr_;
    size_t size_;

public:
    JITProgram(size_t size) :
      program_((uint8_t*) mmap(0, size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0)),
      ptr_(program_),
      size_(size) {} // MAP_FAILED

    ~JITProgram() {
        munmap(program_, size_); // -1
    }

    uint8_t* get_ptr () const { return ptr_; }
    void set_ptr (uint8_t* ptr) { ptr_ = ptr; }

    void writeb(uint8_t byte) {
        (*ptr_++) = byte;
    }

    void writel(uint32_t value) {
        writes((uint8_t*)&value, 4);
    }

    void writes(const uint8_t* bytes, uint32_t size) {
        memcpy(ptr_, bytes, size);
        ptr_ += size;
    }

    void start() {
        // 0000000000000070 break:
        //       70: cc                            int3
        // writeb(0xcc);
    }

    void finish() {
        // 0000000000000071 finish:
        //       71: c3                            retq
        writeb(0xc3);
    }

    void run() {
        uint32_t memory_[30000];
        memset(memory_, 0x00, sizeof(memory_));

        make_memory_executable(program_, size_);

        // asm("int $3");

        // http://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html
        asm("movq %0, %%rdi"
            :
            : "r"(&memory_)
            : "%rdi");

        ((void (*)())program_)();
    }
};

class JITCompiler : public ExpressionVisitor
{
private:
    JITProgram &program_;
    std::vector<uint8_t*> stack;

public:
    JITCompiler(JITProgram& program) : program_(program), stack() {}

    virtual void visit(const Increment& inc) {
        // 0000000000000000 increment:
        //        0: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //        7: 01 07                         addl    %eax, (%rdi)
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(inc.offset());
        program_.writes((uint8_t*)"\x01\x07", 2);        
    }

    virtual void visit(const Decrement& dec) {
        // 0000000000000009 decrement:
        //        9: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       10: 29 07                         subl    %eax, (%rdi)
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(dec.offset());
        program_.writes((uint8_t*)"\x29\x07", 2);
    }

    virtual void visit(const Forward& fwd) {
        // 0000000000000012 forward:
        //       12: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       19: 48 01 c7                      addq    %rax, %rdi
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(fwd.offset()*4);
        program_.writes((uint8_t*)"\x48\x01\xc7", 3);
    }

    virtual void visit(const Backward& bwd) {
        // 000000000000001c backward:
        //       1c: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       23: 48 29 c7                      subq    %rax, %rdi
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(bwd.offset()*4);
        program_.writes((uint8_t*)"\x48\x29\xc7", 3);
    }

    virtual void visit(const Input&) {
        // 0000000000000026 read:
        //       26: 57                            pushq   %rdi
        //       27: 48 c7 c0 03 00 00 02          movq    $33554435, %rax
        //       2e: 48 89 fe                      movq    %rdi, %rsi
        //       31: 48 c7 c7 01 00 00 00          movq    $1, %rdi
        //       38: 48 c7 c2 01 00 00 00          movq    $1, %rdx
        //       3f: 0f 05                         syscall
        //       41: 5f                            popq    %rdi        

        program_.writeb(0x57);
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(0x02000003);
        program_.writes((uint8_t*)"\x48\x89\xfe", 3);
        program_.writes((uint8_t*)"\x48\xc7\xc7\x00\x00\x00\x00", 7);
        program_.writes((uint8_t*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((uint8_t*)"\x0f\x05", 2);
        program_.writeb(0x5f);
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

        program_.writeb(0x57);
        program_.writes((uint8_t*)"\x48\xc7\xc0", 3);
        program_.writel(0x02000004);
        program_.writes((uint8_t*)"\x48\x89\xfe", 3);
        program_.writes((uint8_t*)"\x48\xc7\xc7\x01\x00\x00\x00", 7);
        program_.writes((uint8_t*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((uint8_t*)"\x0f\x05", 2);
        program_.writeb(0x5f);
    }

    virtual void visit(const Loop& loop) {
        // 000000000000005e loop_start:
        //       5e: 83 3f 00                      cmpl    $0, (%rdi)
        //       61: 0f 84 00 00 00 00             je  0
        program_.writes((uint8_t*)"\x83\x3f\x00", 3);
        program_.writes((uint8_t*)"\x0f\x84", 2);
        program_.writel(0); // reserve 4 bytes

        // push current position to stack
        stack.push_back(program_.get_ptr());

        // recurse into subexpressions:
        for(const auto &child: loop.children()) {
            child->accept(*this);
        }

        // recover last position
        uint8_t* after_loop_start = stack.back();
        stack.pop_back();

        // 0000000000000067 loop_end:
        //       67: 83 3f 00                      cmpl    $0, (%rdi)
        //       6a: 0f 85 00 00 00 00             jne 0
        program_.writes((uint8_t*)"\x83\x3f\x00", 3);
        program_.writes((uint8_t*)"\x0f\x85", 2);
        // calculate how much to jump back (consider the 4 bytes
        // of the operand itself):
        uint32_t jump_back = after_loop_start - program_.get_ptr() - 4;
        // append the distance to the jump:
        program_.writel(jump_back);

        // now calculate how much to jump forward in case we want
        // to skip the loop, to fill the pending jump:
        uint8_t *after_loop_end = program_.get_ptr();
        uint32_t jump_fwd = after_loop_end - after_loop_start;

        // go back to the original position (-4), fill the pending
        // jump distance and get back to the the current pos:
        program_.set_ptr(after_loop_start - 4);
        program_.writel(jump_fwd);
        program_.set_ptr(after_loop_end);
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

        JITProgram jit_program(1000000);
        JITCompiler compiler(jit_program);
        compiler.compile(expressions);

        jit_program.run();

    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
