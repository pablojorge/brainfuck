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

    unsigned char* ptr () const { return ptr_; }
    void ptr (unsigned char* ptr) { ptr_ = ptr; }

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

    void movl_rax(ssize_t value) {
        // 100000e7d: 48 c7 c0 0f 00 00 00         movq    $15, %rax
        writes((unsigned char*)"\x48\xc7\xc0", 3);
        writel(value);
    }

    void addl_eax_prdi() {
        // 100000ea3: 01 07                        addl    %eax, (%rdi)
        writes((unsigned char*)"\x01\x07", 2);        
    }

    void int3() {
        writeb(0xcc);
    }

    void ret() {
        writeb(0xc3);
    }

    void start() {
        // int3();
    }

    void finish() {
        ret();
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
        program_.movl_rax(inc.offset());
        program_.addl_eax_prdi();
    }
    virtual void visit(const Decrement& dec) {
        program_.movl_rax(dec.offset());
        // 100000ea5: 29 07                        subl    %eax, (%rdi)
        program_.writes((unsigned char*)"\x29\x07", 2);
    }
    virtual void visit(const Forward& fwd) {
        program_.movl_rax(fwd.offset()*4);
        // 100000e9d: 48 01 c7                     addq    %rax, %rdi
        program_.writes((unsigned char*)"\x48\x01\xc7", 3);
    }
    virtual void visit(const Backward& bwd) {
        program_.movl_rax(bwd.offset()*4);
        // 100000ea0: 48 29 c7                     subq    %rax, %rdi
        program_.writes((unsigned char*)"\x48\x29\xc7", 3);
    }
    virtual void visit(const Input&) {
        // mov rax, 0x2000004     ; sys_write call identifier
        // mov rdi, 1             ; STDOUT file descriptor
        // mov rsi, myMessage     ; buffer to print
        // mov rdx, myMessageLen  ; length of buffer
        // syscall                ; make the system call

        program_.writeb(0x57);
        program_.movl_rax(0x02000003);
        program_.writes((unsigned char*)"\x48\x89\xfe", 3);
        program_.writes((unsigned char*)"\x48\xc7\xc7\x00\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x0f\x05", 2);
        program_.writeb(0x5f);
    }
    virtual void visit(const Output&) {
        // $ objdump -d /usr/lib/system/libsystem_kernel.dylib
        // _read:
        // 1814:       b8 03 00 00 02  movl    $33554435, %eax
        // _write:
        // 3bec:       b8 04 00 00 02  movl    $33554436, %eax

        // 100000efd: 57                           pushq   %rdi
        // 100000efe: 48 c7 c0 04 00 00 02         movq    $33554436, %rax # SYS_write
        // 100000f0c: 48 89 fe                     movq    %rdi, %rsi # buf
        // 100000f05: 48 c7 c7 01 00 00 00         movq    $1, %rdi # fd
        // 100000f0f: 48 c7 c2 01 00 00 00         movq    $1, %rdx # buf_len
        // 100000f16: 0f 05                        syscall
        // 100000f18: 5f                           popq    %rdi

        program_.writeb(0x57);
        program_.movl_rax(0x02000004);
        program_.writes((unsigned char*)"\x48\x89\xfe", 3);
        program_.writes((unsigned char*)"\x48\xc7\xc7\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x48\xc7\xc2\x01\x00\x00\x00", 7);
        program_.writes((unsigned char*)"\x0f\x05", 2);
        program_.writeb(0x5f);
    }
    virtual void visit(const Loop& loop) {
        // 100000f27: 83 3f 00                     cmpl    $0, (%rdi)
        program_.writes((unsigned char*)"\x83\x3f\x00", 3);
        // 100000f6b: 0f 84 2f 00 00 00            je      47 <next_iter>
        program_.writes((unsigned char*)"\x0f\x84", 2);
        program_.writel(0); // TBD

        stack.push_back(program_.ptr());

        this->visit(loop.children());

        // 100000f27: 83 3f 00                     cmpl    $0, (%rdi)
        program_.writes((unsigned char*)"\x83\x3f\x00", 3);
        // 100000f2a: 0f 85 70 00 00 00            jne     112 <next_iter>
        program_.writes((unsigned char*)"\x0f\x85", 2);
        // asm("int $3");
        unsigned char* after_loop_start = stack.back();
        stack.pop_back();

        ssize_t jump_back = after_loop_start - program_.ptr() - 4; // account for operand
        program_.writel(jump_back);

        unsigned char *after_loop_end = program_.ptr();
        ssize_t jump_fwd = after_loop_end - after_loop_start;
        program_.ptr(after_loop_start - 4);
        program_.writel(jump_fwd);
        program_.ptr(after_loop_end);
    }
    virtual void visit(const ExpressionVector& expressions) {
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
