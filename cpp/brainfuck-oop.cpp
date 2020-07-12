#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <tuple>
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

    inline T read() {
        return *this->ptr_;
    }
    inline void write(T c) {
        *this->ptr_=c;
    }
};

class Runner;

class ExpressionBase
{
public:
    ExpressionBase() {}
    virtual ~ExpressionBase() {}

    virtual void run(Runner& runner) const = 0;
};

using ExpressionVector = std::vector<std::unique_ptr<ExpressionBase>>;

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

class Increment : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Increment(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Increment() = default;
    virtual void run(Runner& runner) const {runner.memory().inc(offset_);}
};

class Decrement : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Decrement(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Decrement() = default;
    virtual void run(Runner& runner) const {runner.memory().dec(offset_);}
};

class Forward : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Forward(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Forward() = default;
    virtual void run(Runner& runner) const {runner.memory().fwd(offset_);}
};

class Backward : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Backward(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Backward() = default;
    virtual void run(Runner& runner) const {runner.memory().bwd(offset_);}
};

class Input : public ExpressionBase
{
public:
    Input() : ExpressionBase() {}

    virtual ~Input() = default;

    virtual void run(Runner& runner) const {
        runner.memory().write(getchar());
        if (runner.memory().read() == EOF)
            exit(0);
    }
};

class Output : public ExpressionBase
{
public:
    Output() : ExpressionBase() {}

    virtual ~Output() = default;

    virtual void run(Runner& runner) const {
        putchar(runner.memory().read()); 
        fflush(stdout); 
    }
};

class Loop : public ExpressionBase
{
private:
    ExpressionVector children_;

public:
    Loop(ExpressionVector &&children)
     : ExpressionBase(),
       children_(std::move(children)) {}

    Loop(const Loop&) = delete;

    virtual ~Loop() = default;

    virtual void run(Runner& runner) const {
        while(runner.memory().read() > 0) {
            runner.run(children_);
        }
    }
};

class Parser
{
    std::vector<char>::iterator pos_, end_;

public:
    Parser(std::vector<char>& tokens)
     : pos_(tokens.begin()),
       end_(tokens.end()) {}

    ~Parser() = default;
    
    ExpressionVector parse();
};

ExpressionVector Parser::parse() {
    ExpressionVector expressions;

    auto push = [&](ExpressionBase* exp) {
        expressions.push_back(
            std::unique_ptr<ExpressionBase>(exp)
        );
    };

    while (pos_ != end_) {
        switch(*pos_) {
            case '+': push(new Increment(1)); break;
            case '-': push(new Decrement(1)); break;
            case '>': push(new Forward(1));   break;
            case '<': push(new Backward(1));  break;
            case ',': push(new Input());      break;
            case '.': push(new Output());     break;
            case '[':
                ++pos_;
                push(new Loop(std::move(parse())));
                break;
            case ']':
                return std::move(expressions);
        }
        ++pos_;
    }

    return std::move(expressions);
}

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

    auto expressions = Parser(program).parse();

    Runner().run(expressions);

    return 0;
}
