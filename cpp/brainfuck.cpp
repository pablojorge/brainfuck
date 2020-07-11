#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <tuple>
#include <vector>

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v);

enum class Token {
    Inc,
    Dec,
    Fwd,
    Bwd,
    Input,
    Output,
    LoopStart,
    LoopEnd
};

std::ostream& operator<<(std::ostream& os, const Token& token)
{
    switch(token) {
        case Token::Inc:        os << "<Inc>"; break;
        case Token::Dec:        os << "<Dec>"; break;
        case Token::Fwd:        os << "<Fwd>"; break;
        case Token::Bwd:        os << "<Bwd>"; break;
        case Token::Input:      os << "<Input>"; break;
        case Token::Output:     os << "<Output>"; break;
        case Token::LoopStart:  os << "<LoopStart>"; break;
        case Token::LoopEnd:    os << "<LoopEnd>"; break;
    }
    return os;
}

using TokenVector = std::vector<Token>;

auto tokenize(const std::vector<char> &program) {
    TokenVector tokens;

    for(auto c: program) {
        switch(c) {
            case '+': tokens.push_back(Token::Inc); break;
            case '-': tokens.push_back(Token::Dec); break;
            case '>': tokens.push_back(Token::Fwd); break;
            case '<': tokens.push_back(Token::Bwd); break;
            case ',': tokens.push_back(Token::Input); break;
            case '.': tokens.push_back(Token::Output); break;
            case '[': tokens.push_back(Token::LoopStart); break;
            case ']': tokens.push_back(Token::LoopEnd); break;
        }
    }

    return std::move(tokens);
}

enum class Operation {
    Inc,
    Dec,
    Fwd,
    Bwd,
    Input,
    Output,
    Loop
};

std::ostream& operator<<(std::ostream& os, const Operation& op)
{
    switch(op) {
        case Operation::Inc:    os << "<Inc>"; break;
        case Operation::Dec:    os << "<Dec>"; break;
        case Operation::Fwd:    os << "<Fwd>"; break;
        case Operation::Bwd:    os << "<Bwd>"; break;
        case Operation::Input:  os << "<Input>"; break;
        case Operation::Output: os << "<Output>"; break;
        case Operation::Loop:   os << "<Loop>"; break;
    }
    return os;
}

class Expression
{
public:
    Operation op_;
    int arg_;
    std::vector<Expression> children_;

    Expression(Operation op,
               int arg,
               const std::vector<Expression> &children)
     : op_(op),
       arg_(arg),
       children_(children) {}

    Expression(Operation op)
     : op_(op),
       arg_(1),
       children_() {}

    // Expression(const Expression&) = delete;

    ~Expression() = default;
};

std::ostream& operator<<(std::ostream& os, const Expression& exp)
{
    os << "Exp(op: " << exp.op_
       << ", arg: " << exp.arg_ 
       << ", children: " << exp.children_
       << ")";
    return os;
}

using ExpressionVector = std::vector<Expression>;

std::tuple<ExpressionVector, TokenVector::iterator>
do_parse(TokenVector::iterator begin,
         TokenVector::iterator end) {
    ExpressionVector expressions;
    std::tuple<ExpressionVector, TokenVector::iterator> ret;

    auto push_unit_op = [&](Operation op) {
        expressions.push_back(Expression(op));
    };

    while (begin != end) {
        switch(*begin) {
            case Token::Inc:
                push_unit_op(Operation::Inc);
                break;
            case Token::Dec:
                push_unit_op(Operation::Dec);
                break;
            case Token::Fwd:
                push_unit_op(Operation::Fwd);
                break;
            case Token::Bwd:
                push_unit_op(Operation::Bwd);
                break;
            case Token::Input:
                push_unit_op(Operation::Input);
                break;
            case Token::Output:
                push_unit_op(Operation::Output);
                break;
            case Token::LoopStart:
                ret = do_parse(begin+1, end);
                expressions.push_back(Expression(
                    Operation::Loop,
                    0,
                    std::get<0>(ret)
                ));
                begin = std::get<1>(ret);
                break;
            case Token::LoopEnd:
                return std::make_tuple(std::move(expressions), begin);
        }
        ++begin;
    }

    return std::make_tuple(std::move(expressions), begin);
}

ExpressionVector parse(TokenVector::iterator begin, TokenVector::iterator end) {
    return std::get<0>(do_parse(begin, end));
}


class Memory
{
    std::array<char, 30000> memory_;
    decltype(memory_.begin()) ptr_;

public:
    Memory() : memory_(), ptr_(memory_.begin()) {}
    ~Memory() = default;

    inline void inc()  {++(*this->ptr_);}
    inline void dec()  {--(*this->ptr_);}
    inline void fwd()  {  ++this->ptr_; }
    inline void bwd()  {  --this->ptr_; }
    inline char read() {
        return *this->ptr_;
    }
    inline void write(char c) {
        *this->ptr_=c;
    }
};

void do_run(const ExpressionVector& expressions, Memory &memory) {
    for(const auto &expression: expressions) {
        switch(expression.op_) {
            case Operation::Inc: memory.inc(); break;
            case Operation::Dec: memory.dec(); break;
            case Operation::Fwd: memory.fwd(); break;
            case Operation::Bwd: memory.bwd(); break;
            case Operation::Input:
                memory.write(getchar());
                if (memory.read() == EOF) exit(0);
                break;
            case Operation::Output:
                putchar(memory.read()); 
                fflush(stdout); 
                break;
            case Operation::Loop:
                while(memory.read() > 0) {
                    do_run(expression.children_, memory);
                }
                break;
        }
    }
}

void run(const ExpressionVector& expressions) {
    Memory memory;
    do_run(expressions, memory);
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T> &v) {
    std::cout << "[";
    if (v.size()) {
        std::for_each(v.begin(), v.end()-1,
                      [](auto &e){std::cout << e << ", ";});
        std::cout << *(v.end()-1);
    }
    std::cout << "]";
    return os;
}

int main(int argc, char *argv[]) {
    std::ifstream ifs(argv[1]);

    if (!ifs.good()) {
        std::cerr << "Invalid filename!" << std::endl;
        return 1;
    }

    std::vector<char> program;

    std::copy(
        (std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()),
        std::back_inserter(program)
    );

    auto tokens = tokenize(program);
    // std::cout << "tokens: " << tokens << std::endl;
    auto expressions = parse(tokens.begin(), tokens.end());
    // std::cout << "expressions: " << expressions << std::endl;

    run(expressions);

    return 0;
}
