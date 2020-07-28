#include <array>
#include <exception>
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