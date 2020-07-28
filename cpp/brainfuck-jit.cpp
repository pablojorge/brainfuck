#include <fstream>
#include <iostream>
#include <vector>

#include <sys/mman.h>
#include <cstring>

#include "brainfuck.h"

void* alloc_writable_memory(size_t size) {
  void* ptr = mmap(0, size,
                   PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  if (ptr == (void*)-1) {
    perror("mmap");
    return NULL;
  }
  return ptr;
}

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
    unsigned char *program_, *ptr_;
    unsigned int mem[30000];

public:
    JITProgram() {
        program_ = (unsigned char*)alloc_writable_memory(1000000);
        ptr_ = program_;
        memset(mem, 0x00, sizeof(mem));
    }

    unsigned char* get_ptr () const { return ptr_; }
    void set_ptr (unsigned char* ptr) { ptr_ = ptr; }

    void writeb(unsigned char byte) {
        (*ptr_++) = byte;
    }

    void writel(ssize_t value) {
        writes((unsigned char*)&value, 4);
    }

    void writes(const unsigned char* bytes, ssize_t size) {
        memcpy(ptr_, bytes, size);
        ptr_ += size;
    }

    void movq_rax(ssize_t value) {
        // 0: 48 c7 c0 01 00 00 00          movq    $1, %rax
        writes((unsigned char*)"\x48\xc7\xc0", 3);
        writel(value);
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
        make_memory_executable(program_, 1000000);

        // asm("int $3");

        asm("movq %0, %%rdi"
            :
            : "r"(&mem)
            : "%rdi");

        ((void (*)())program_)();
    }
};

class JITVisitor : public ExpressionVisitor
{
private:
    JITProgram &program_;
    std::vector<unsigned char*> stack;

public:
    JITVisitor(JITProgram& program) : program_(program), stack() {}

    virtual void visit(const Increment& inc) {
        // 0000000000000000 increment:
        //        0: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //        7: 01 07                         addl    %eax, (%rdi)
        program_.movq_rax(inc.offset());
        program_.writes((unsigned char*)"\x01\x07", 2);        
    }

    virtual void visit(const Decrement& dec) {
        // 0000000000000009 decrement:
        //        9: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       10: 29 07                         subl    %eax, (%rdi)
        program_.movq_rax(dec.offset());
        program_.writes((unsigned char*)"\x29\x07", 2);
    }

    virtual void visit(const Forward& fwd) {
        // 0000000000000012 forward:
        //       12: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       19: 48 01 c7                      addq    %rax, %rdi
        program_.movq_rax(fwd.offset()*4);
        program_.writes((unsigned char*)"\x48\x01\xc7", 3);
    }

    virtual void visit(const Backward& bwd) {
        // 000000000000001c backward:
        //       1c: 48 c7 c0 01 00 00 00          movq    $1, %rax
        //       23: 48 29 c7                      subq    %rax, %rdi
        program_.movq_rax(bwd.offset()*4);
        program_.writes((unsigned char*)"\x48\x29\xc7", 3);
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
        program_.movq_rax(0x02000003);
        program_.writes((unsigned char*)"\x48\x89\xfe", 3);
        program_.writes((unsigned char*)"\x48\xc7\xc7\x00\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x0f\x05", 2);
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
        program_.movq_rax(0x02000004);
        program_.writes((unsigned char*)"\x48\x89\xfe", 3);
        program_.writes((unsigned char*)"\x48\xc7\xc7\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x0f\x05", 2);
        program_.writeb(0x5f);
    }

    virtual void visit(const Loop& loop) {
        // 000000000000005e loop_start:
        //       5e: 83 3f 00                      cmpl    $0, (%rdi)
        //       61: 0f 84 00 00 00 00             je  0
        program_.writes((unsigned char*)"\x83\x3f\x00", 3);
        program_.writes((unsigned char*)"\x0f\x84", 2);
        program_.writel(0); // reserve 4 bytes

        // push current position to stack
        stack.push_back(program_.get_ptr());

        // recurse into subexpressions:
        this->visit(loop.children());

        // recover last position
        unsigned char* after_loop_start = stack.back();
        stack.pop_back();

        // 0000000000000067 loop_end:
        //       67: 83 3f 00                      cmpl    $0, (%rdi)
        //       6a: 0f 85 00 00 00 00             jne 0
        program_.writes((unsigned char*)"\x83\x3f\x00", 3);
        program_.writes((unsigned char*)"\x0f\x85", 2);
        // calculate how much to jump back (consider the 4 bytes
        // of the operand itself):
        ssize_t jump_back = after_loop_start - program_.get_ptr() - 4;
        // append the distance to the jump:
        program_.writel(jump_back);

        // now calculate how much to jump forward in case we want
        // to skip the loop, to fill the pending jump:
        unsigned char *after_loop_end = program_.get_ptr();
        ssize_t jump_fwd = after_loop_end - after_loop_start;

        // go back to the original position (-4), fill the pending
        // jump distance and get back to the the current pos:
        program_.set_ptr(after_loop_start - 4);
        program_.writel(jump_fwd);
        program_.set_ptr(after_loop_end);
    }

    void visit(const ExpressionVector& expressions) {
        for(const auto &expression: expressions) {
            expression->accept(*this);
        }
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
        JITProgram jit_program;
        jit_program.start();
        JITVisitor visitor(jit_program);
        visitor.visit(expressions);
        jit_program.finish();
        jit_program.run();
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
