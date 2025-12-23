#ifndef CPYTHON_CPP_COMBINATORS_HPP
#define CPYTHON_CPP_COMBINATORS_HPP

#include "tokenizer.hpp"
#include <optional>
#include <vector>
#include <functional>
#include <variant>
#include <concepts>

namespace cpython_cpp {
namespace parser {

/**
 * Template-based parser combinators
 * 
 * Provides functional programming style parser composition:
 * - sequence: parse A then B
 * - choice: parse A or B
 * - many: parse zero or more
 * - optional: parse zero or one
 * 
 * All combinators use templates for zero-cost abstractions
 */

// Forward declarations
template<typename T>
class ParseResult;

template<typename T>
class Parser;

// Parse result - either success with value or failure
template<typename T>
class ParseResult {
public:
    std::optional<T> value;
    size_t consumed;  // Number of tokens consumed
    std::string error;
    
    ParseResult() : consumed(0) {}
    ParseResult(T v, size_t c) : value(std::move(v)), consumed(c) {}
    ParseResult(std::string err) : error(std::move(err)), consumed(0) {}
    
    bool success() const { return value.has_value(); }
    bool failed() const { return !value.has_value(); }
    
    // Monadic bind operation
    template<typename F>
    auto bind(F&& f) -> ParseResult<decltype(f(std::declval<T>()).value.value())> {
        using U = decltype(f(std::declval<T>()).value.value());
        if (failed()) {
            return ParseResult<U>(error);
        }
        auto result = f(*value);
        result.consumed += consumed;
        return result;
    }
    
    // Functor map operation
    template<typename F>
    auto map(F&& f) -> ParseResult<decltype(f(std::declval<T>()))> {
        using U = decltype(f(std::declval<T>()));
        if (failed()) {
            return ParseResult<U>(error);
        }
        return ParseResult<U>(f(*value), consumed);
    }
};

// Parser concept
template<typename P, typename Input>
concept ParserConcept = requires(P p, Input& input) {
    { p.parse(input) };
};

// Base parser class template
template<typename T>
class Parser {
public:
    using value_type = T;
    using result_type = ParseResult<T>;
    
    virtual ~Parser() = default;
    virtual result_type parse(std::vector<Token>& tokens, size_t& pos) const = 0;
};

// Token matcher parser - matches a specific token type
template<TokenType Type>
class TokenParser : public Parser<TypedToken<Type>> {
public:
    using result_type = ParseResult<TypedToken<Type>>;
    
    result_type parse(std::vector<Token>& tokens, size_t& pos) const override {
        if (pos >= tokens.size()) {
            return result_type("Unexpected end of input");
        }
        
        if (auto typed = tokens[pos].template as<Type>()) {
            pos++;
            return result_type(*typed, 1);
        }
        
        return result_type("Expected token type");
    }
};

// Sequence combinator - parse A then B
template<typename P1, typename P2>
class SequenceParser : public Parser<std::pair<typename P1::value_type, typename P2::value_type>> {
public:
    using T1 = typename P1::value_type;
    using T2 = typename P2::value_type;
    using result_type = ParseResult<std::pair<T1, T2>>;
    
    SequenceParser(P1 p1, P2 p2) : parser1_(std::move(p1)), parser2_(std::move(p2)) {}
    
    result_type parse(std::vector<Token>& tokens, size_t& pos) const override {
        size_t start_pos = pos;
        
        auto r1 = parser1_.parse(tokens, pos);
        if (r1.failed()) {
            pos = start_pos;
            return result_type(r1.error);
        }
        
        auto r2 = parser2_.parse(tokens, pos);
        if (r2.failed()) {
            pos = start_pos;
            return result_type(r2.error);
        }
        
        return result_type(
            std::make_pair(*r1.value, *r2.value),
            r1.consumed + r2.consumed
        );
    }
    
private:
    P1 parser1_;
    P2 parser2_;
};

// Choice combinator - parse A or B
template<typename P1, typename P2>
class ChoiceParser : public Parser<std::variant<typename P1::value_type, typename P2::value_type>> {
public:
    using T1 = typename P1::value_type;
    using T2 = typename P2::value_type;
    using result_type = ParseResult<std::variant<T1, T2>>;
    
    ChoiceParser(P1 p1, P2 p2) : parser1_(std::move(p1)), parser2_(std::move(p2)) {}
    
    result_type parse(std::vector<Token>& tokens, size_t& pos) const override {
        size_t start_pos = pos;
        
        auto r1 = parser1_.parse(tokens, pos);
        if (r1.success()) {
            return result_type(std::variant<T1, T2>(*r1.value), r1.consumed);
        }
        
        pos = start_pos;
        auto r2 = parser2_.parse(tokens, pos);
        if (r2.success()) {
            return result_type(std::variant<T1, T2>(*r2.value), r2.consumed);
        }
        
        pos = start_pos;
        return result_type("No alternative matched");
    }
    
private:
    P1 parser1_;
    P2 parser2_;
};

// Many combinator - parse zero or more
template<typename P>
class ManyParser : public Parser<std::vector<typename P::value_type>> {
public:
    using T = typename P::value_type;
    using result_type = ParseResult<std::vector<T>>;
    
    explicit ManyParser(P p) : parser_(std::move(p)) {}
    
    result_type parse(std::vector<Token>& tokens, size_t& pos) const override {
        std::vector<T> results;
        size_t total_consumed = 0;
        
        while (true) {
            size_t start_pos = pos;
            auto r = parser_.parse(tokens, pos);
            
            if (r.failed()) {
                pos = start_pos;
                break;
            }
            
            results.push_back(*r.value);
            total_consumed += r.consumed;
        }
        
        return result_type(std::move(results), total_consumed);
    }
    
private:
    P parser_;
};

// Optional combinator - parse zero or one
template<typename P>
class OptionalParser : public Parser<std::optional<typename P::value_type>> {
public:
    using T = typename P::value_type;
    using result_type = ParseResult<std::optional<T>>;
    
    explicit OptionalParser(P p) : parser_(std::move(p)) {}
    
    result_type parse(std::vector<Token>& tokens, size_t& pos) const override {
        size_t start_pos = pos;
        auto r = parser_.parse(tokens, pos);
        
        if (r.success()) {
            return result_type(std::optional<T>(*r.value), r.consumed);
        }
        
        pos = start_pos;
        return result_type(std::optional<T>(), 0);
    }
    
private:
    P parser_;
};

// Combinator factory functions (template deduction)

template<TokenType Type>
auto token() {
    return TokenParser<Type>();
}

template<typename P1, typename P2>
auto sequence(P1 p1, P2 p2) {
    return SequenceParser<P1, P2>(std::move(p1), std::move(p2));
}

template<typename P1, typename P2>
auto choice(P1 p1, P2 p2) {
    return ChoiceParser<P1, P2>(std::move(p1), std::move(p2));
}

template<typename P>
auto many(P p) {
    return ManyParser<P>(std::move(p));
}

template<typename P>
auto optional(P p) {
    return OptionalParser<P>(std::move(p));
}

// Operator overloads for combinator composition

template<typename P1, typename P2>
auto operator>>(P1 p1, P2 p2) {
    return sequence(std::move(p1), std::move(p2));
}

template<typename P1, typename P2>
auto operator|(P1 p1, P2 p2) {
    return choice(std::move(p1), std::move(p2));
}

// Variadic choice combinator (choice among multiple parsers)
template<typename... Parsers>
class VariadicChoiceParser;

template<typename P>
class VariadicChoiceParser<P> : public P {
public:
    explicit VariadicChoiceParser(P p) : P(std::move(p)) {}
};

template<typename P1, typename... Rest>
class VariadicChoiceParser<P1, Rest...> {
public:
    using first_type = typename P1::value_type;
    using rest_parser = VariadicChoiceParser<Rest...>;
    
    VariadicChoiceParser(P1 p1, Rest... rest)
        : first_(std::move(p1)), rest_(std::move(rest)...) {}
    
    auto parse(std::vector<Token>& tokens, size_t& pos) const {
        size_t start_pos = pos;
        
        auto r1 = first_.parse(tokens, pos);
        if (r1.success()) {
            return r1;
        }
        
        pos = start_pos;
        return rest_.parse(tokens, pos);
    }
    
private:
    P1 first_;
    rest_parser rest_;
};

template<typename... Parsers>
auto choice_of(Parsers... parsers) {
    return VariadicChoiceParser<Parsers...>(std::move(parsers)...);
}

} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_COMBINATORS_HPP
