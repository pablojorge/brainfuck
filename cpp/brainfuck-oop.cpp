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

class Increment;
class Decrement;
class Forward;
class Backward;
class Input;
class Output;
class Loop;

class BaseMatcher
{
public:
    virtual bool match(const Increment&) const {return false;}
    virtual bool match(const Decrement&) const {return false;}
    virtual bool match(const Forward&) const {return false;}
    virtual bool match(const Backward&) const {return false;}
    virtual bool match(const Input&) const {return false;}
    virtual bool match(const Output&) const {return false;}
    virtual bool match(const Loop&) const {return false;}
};

template <typename T>
class ConcreteMatcher : public BaseMatcher
{
    virtual bool match(const T&) const {return true;}
};

using MatcherPtr = std::unique_ptr<BaseMatcher>;

class Runner;
class ExpressionBase
{
public:
    ExpressionBase() {}
    virtual ~ExpressionBase() {}

    virtual void run(Runner& runner) const = 0;
    virtual void extend() = 0;
    virtual bool matches(const ExpressionBase& other) const  = 0;
    virtual MatcherPtr matcher() const = 0;
};

using ExpressionPtr = std::unique_ptr<ExpressionBase>;
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

class Increment : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Increment(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Increment() = default;
    virtual void run(Runner& runner) const {runner.memory().inc(offset_);}
    virtual void extend() {++offset_;}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new ConcreteMatcher<Increment>());
    }
    virtual bool matches(const ExpressionBase& other) const {
        return other.matcher()->match(*this);
    }
};

class Decrement : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Decrement(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Decrement() = default;
    virtual void run(Runner& runner) const {runner.memory().dec(offset_);}
    virtual void extend() {++offset_;}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new ConcreteMatcher<Decrement>());
    }
    virtual bool matches(const ExpressionBase& other) const {
        return other.matcher()->match(*this);
    }
};

class Forward : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Forward(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Forward() = default;
    virtual void run(Runner& runner) const {runner.memory().fwd(offset_);}
    virtual void extend() {++offset_;}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new ConcreteMatcher<Forward>());
    }
    virtual bool matches(const ExpressionBase& other) const {
        return other.matcher()->match(*this);
    }
};

class Backward : public ExpressionBase
{
private:
    ssize_t offset_;
public:
    Backward(ssize_t offset) : ExpressionBase(), offset_(offset) {}
    virtual ~Backward() = default;
    virtual void run(Runner& runner) const {runner.memory().bwd(offset_);}
    virtual void extend() {++offset_;}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new ConcreteMatcher<Backward>());
    }
    virtual bool matches(const ExpressionBase& other) const {
        return other.matcher()->match(*this);
    }
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

    virtual void extend() {}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new BaseMatcher());
    }
    virtual bool matches(const ExpressionBase& other) const {return false;}
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

    virtual void extend() {}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new BaseMatcher());
    }
    virtual bool matches(const ExpressionBase& other) const {return false;}
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

    virtual void extend() {}
    virtual MatcherPtr matcher() const {
        return MatcherPtr(new BaseMatcher());
    }
    virtual bool matches(const ExpressionBase& other) const {return false;}
};

using TokenVector = std::vector<char>;

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
                next = ExpressionPtr(new Loop(std::move(*expressions)));
                expressions = std::move(stack.back());
                stack.pop_back();
                break;
        }

        if (!next) {
            continue;
        } else if (expressions->empty() ||
                  !expressions->back()->matches(*next)) {
            expressions->push_back(std::move(next));
        } else {
            expressions->back()->extend();
        }
    }

    return std::move(*expressions);
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

    auto expressions = Parser().parse(program);

    Runner().run(expressions);

    return 0;
}
