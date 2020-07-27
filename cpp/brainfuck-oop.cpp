#include <array>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

template <typename T=unsigned int>
class Memory 
{
private:
    std::array<T, 30000> memory_;
    typename decltype(memory_)::iterator ptr_;

public:
    Memory() : memory_(), ptr_(memory_.begin()) {}
    ~Memory() = default;

    inline void inc(T offset)       { *this->ptr_ += offset; }
    inline void dec(T offset)       { *this->ptr_ -= offset; }
    inline void fwd(ssize_t offset) {  this->ptr_ += offset; }
    inline void bwd(ssize_t offset) {  this->ptr_ -= offset; }

    inline T read() const { return *this->ptr_; }
    inline void write(T c) { *this->ptr_=c; }
};

class Increment;
class Decrement;
class Forward;
class Backward;
class Input;
class Output;
class Loop;

class ExpressionVisitor
{
public:
    virtual void visit(const Increment&) = 0;
    virtual void visit(const Decrement&) = 0;
    virtual void visit(const Forward&) = 0;
    virtual void visit(const Backward&) = 0;
    virtual void visit(const Input&) = 0;
    virtual void visit(const Output&) = 0;
    virtual void visit(const Loop&) = 0;
};

class Runner;
class Expression
{
public:
    Expression() {}
    virtual ~Expression() {}

    virtual void run(Runner& runner) const = 0;
    virtual void accept(ExpressionVisitor& visitor) const = 0;
    virtual bool repeatable() const {return false;}
    virtual void repeat() {}
};

using ExpressionPtr = std::unique_ptr<Expression>;
using ExpressionVector = std::vector<ExpressionPtr>;

class Runner {
private:
    Memory<> memory_;

public:
    Runner() = default;
    ~Runner() = default;

    inline Memory<>& memory() {return memory_;};

    void run(const ExpressionVector& expressions) {
        for(const auto &expression: expressions) {
            expression->run(*this);
        }
    }
};

class Increment : public Expression
{
private:
    ssize_t offset_;
public:
    Increment(ssize_t offset) : Expression(), offset_(offset) {}
    virtual void run(Runner& runner) const {runner.memory().inc(offset_);}
    virtual void accept(ExpressionVisitor& visitor) const {visitor.visit(*this);}
    virtual bool repeatable() const {return true;}
    virtual void repeat() {++offset_;}
    ssize_t offset() const {return offset_;}
};

class Decrement : public Expression
{
private:
    ssize_t offset_;
public:
    Decrement(ssize_t offset) : Expression(), offset_(offset) {}
    virtual void run(Runner& runner) const {runner.memory().dec(offset_);}
    virtual void accept(ExpressionVisitor& visitor) const {visitor.visit(*this);}
    virtual bool repeatable() const {return true;}
    virtual void repeat() {++offset_;}
    ssize_t offset() const {return offset_;}
};

class Forward : public Expression
{
private:
    ssize_t offset_;
public:
    Forward(ssize_t offset) : Expression(), offset_(offset) {}
    virtual void run(Runner& runner) const {runner.memory().fwd(offset_);}
    virtual void accept(ExpressionVisitor& visitor) const {visitor.visit(*this);}
    virtual bool repeatable() const {return true;}
    virtual void repeat() {++offset_;}
    ssize_t offset() const {return offset_;}
};

class Backward : public Expression
{
private:
    ssize_t offset_;
public:
    Backward(ssize_t offset) : Expression(), offset_(offset) {}
    virtual void run(Runner& runner) const {runner.memory().bwd(offset_);}
    virtual void accept(ExpressionVisitor& visitor) const {visitor.visit(*this);}
    virtual bool repeatable() const {return true;}
    virtual void repeat() {++offset_;}
    ssize_t offset() const {return offset_;}
};

class Input : public Expression
{
public:
    virtual void run(Runner& runner) const {
        runner.memory().write(getchar());
        if (runner.memory().read() == EOF)
            exit(0);
    }

    virtual void accept(ExpressionVisitor& visitor) const {
        visitor.visit(*this);
    }
};

class Output : public Expression
{
public:
    virtual void run(Runner& runner) const {
        putchar(runner.memory().read()); 
        fflush(stdout); 
    }

    virtual void accept(ExpressionVisitor& visitor) const {
        visitor.visit(*this);
    }
};

class Loop : public Expression
{
private:
    ExpressionVector children_;

public:
    Loop(ExpressionVector &&children)
     : Expression(),
       children_(std::move(children)) {}

    Loop(const Loop&) = delete;

    const ExpressionVector& children() const {return children_;}

    virtual void run(Runner& runner) const {
        while(runner.memory().read() > 0) {
            runner.run(children_);
        }
    }

    virtual void accept(ExpressionVisitor& visitor) const {
        visitor.visit(*this);
    }
};

using TokenVector = std::vector<char>;

class ExcessiveOpeningBrackets: public std::exception {
    virtual const char* what() const throw() {
        return "Too many opening brackets";
    }
};

class UnexpectedClosingBracket: public std::exception {
    virtual const char* what() const throw() {
        return "Too many closing brackets";
    }
};

class Parser
{
public:
    Parser() = default;
    ~Parser() = default;
    
    ExpressionVector parse(TokenVector&);
};

ExpressionVector Parser::parse(TokenVector& tokens) {
    using ExpressionVectorPtr = std::unique_ptr<ExpressionVector>;

    std::vector<ExpressionVectorPtr> stack;
    ExpressionVectorPtr expressions(new ExpressionVector());

    for (auto token: tokens) {
        ExpressionPtr next;

        switch(token) {
            case '+': next = ExpressionPtr(new Increment(1)); break;
            case '-': next = ExpressionPtr(new Decrement(1)); break;
            case '>': next = ExpressionPtr(new Forward(1));   break;
            case '<': next = ExpressionPtr(new Backward(1));  break;
            case ',': next = ExpressionPtr(new Input());      break;
            case '.': next = ExpressionPtr(new Output());     break;
            case '[':
                stack.push_back(std::move(expressions));
                expressions = ExpressionVectorPtr(new ExpressionVector());
                break;
            case ']':
                if (stack.empty()) throw UnexpectedClosingBracket();
                next = ExpressionPtr(new Loop(std::move(*expressions)));
                expressions = std::move(stack.back());
                stack.pop_back();
                break;
        }

        if (!next) {
            continue;
        } else if (expressions->empty() ||
                  typeid(*expressions->back()) != typeid(*next) ||
                  !expressions->back()->repeatable()) {
            expressions->push_back(std::move(next));
        } else {
            expressions->back()->repeat();
        }
    }

    if (!stack.empty()) throw ExcessiveOpeningBrackets();

    return std::move(*expressions);
}

#include <sys/mman.h>

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
    unsigned char output[1000000];

public:
    JITProgram() {
        program_ = (unsigned char*)alloc_writable_memory(1000000);
        ptr_ = program_;
        memset(mem, 0x00, sizeof(mem));
        memset(output, 0x00, sizeof(output));
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

        asm("movq %0, %%rsi"
            :
            : "r"(&output)
            : "%rsi");

        ((void (*)())program_)();

        // asm("int $3");
        std::cout << (char*) output;
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
    }
    virtual void visit(const Output&) {
        // 100000f12: 8a 27                        movb    (%rdi), %ah
        program_.writes((unsigned char*)"\x8a\x27", 2);
        // 100000f14: 88 26                        movb    %ah, (%rsi)
        program_.writes((unsigned char*)"\x88\x26", 2);
        // 100000f16: 48 ff c6                     incq    %rsi
        program_.writes((unsigned char*)"\x48\xff\xc6", 3);
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
        // Runner().run(expressions);
        JITProgram jit_program;
        jit_program.start();
        JITVisitor visitor(jit_program);
        visitor.visit(expressions);
        jit_program.finish();
        jit_program.run();
        program.push_back(0x00);
    } catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}
