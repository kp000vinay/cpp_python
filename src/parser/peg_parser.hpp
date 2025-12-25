#ifndef CPYTHON_CPP_PEG_PARSER_HPP
#define CPYTHON_CPP_PEG_PARSER_HPP

/**
 * PEG Parser - Template-based parser following CPython's architecture
 * 
 * Key features:
 * - Memoization for O(n) parsing complexity
 * - Lookahead operators (&, !)
 * - Cut operator (~) for committed alternatives
 * - Left recursion support
 * - Type-safe AST generation
 * 
 * Reference: CPython's Parser/pegen.c, Grammar/python.gram
 */

#include "tokenizer_v2.hpp"
#include "../ast/module.hpp"
#include "../ast/stmt.hpp"
#include "../ast/expr.hpp"
#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <optional>
#include <variant>
#include <tuple>
#include <string_view>
#include <type_traits>
#include <any>

namespace cpython_cpp {
namespace parser {

// Forward declarations
template<typename T>
struct ParseResult;

class PEGParser;

// ============================================================================
// Parse Result - Success/Failure with optional value
// ============================================================================

template<typename T>
struct ParseResult {
    std::optional<T> value;
    bool committed = false;  // True if cut operator was encountered
    
    constexpr ParseResult() = default;
    constexpr ParseResult(T v) : value(std::move(v)) {}
    constexpr ParseResult(std::nullopt_t) : value(std::nullopt) {}
    
    constexpr bool success() const noexcept { return value.has_value(); }
    constexpr bool failed() const noexcept { return !value.has_value(); }
    constexpr explicit operator bool() const noexcept { return success(); }
    
    constexpr T& operator*() { return *value; }
    constexpr const T& operator*() const { return *value; }
    constexpr T* operator->() { return &*value; }
    constexpr const T* operator->() const { return &*value; }
    
    // Mark as committed (cut operator encountered)
    constexpr ParseResult& commit() noexcept {
        committed = true;
        return *this;
    }
    
    // Transform the result value
    template<typename F>
    constexpr auto map(F&& f) const -> ParseResult<std::invoke_result_t<F, const T&>> {
        using U = std::invoke_result_t<F, const T&>;
        if (success()) {
            return ParseResult<U>(f(*value));
        }
        return ParseResult<U>();
    }
};

// ============================================================================
// Memoization Cache Entry
// ============================================================================

struct MemoKey {
    size_t position;
    const void* rule_id;  // Pointer to rule function for identification
    
    bool operator==(const MemoKey& other) const noexcept {
        return position == other.position && rule_id == other.rule_id;
    }
};

struct MemoKeyHash {
    size_t operator()(const MemoKey& key) const noexcept {
        return std::hash<size_t>{}(key.position) ^ 
               (std::hash<const void*>{}(key.rule_id) << 1);
    }
};

struct MemoEntry {
    std::any value;
    size_t end_position;
    bool success;
};

// ============================================================================
// Parser State - Tracks position and memoization
// ============================================================================

class ParserState {
public:
    std::vector<Token> tokens;
    size_t position = 0;
    std::unordered_map<MemoKey, MemoEntry, MemoKeyHash> memo_cache;
    
    // Location tracking
    int start_lineno = 1;
    int start_col_offset = 0;
    int end_lineno = 1;
    int end_col_offset = 0;
    
    explicit ParserState(std::vector<Token> toks) 
        : tokens(std::move(toks)) {}
    
    // Get current token
    const Token& current() const noexcept {
        if (position >= tokens.size()) {
            return tokens.back();  // Return EOF token
        }
        return tokens[position];
    }
    
    // Peek at next token
    const Token& peek(size_t offset = 1) const noexcept {
        size_t idx = position + offset;
        if (idx >= tokens.size()) {
            return tokens.back();
        }
        return tokens[idx];
    }
    
    // Check if at end
    bool at_end() const noexcept {
        return position >= tokens.size() || 
               tokens[position].type == TokenType::END_OF_FILE;
    }
    
    // Advance position
    void advance() noexcept {
        if (position < tokens.size()) {
            position++;
        }
    }
    
    // Get/set mark for backtracking
    size_t mark() const noexcept { return position; }
    void reset(size_t pos) noexcept { position = pos; }
    
    // Update location info from current token
    void update_location() {
        if (position < tokens.size()) {
            const auto& tok = tokens[position];
            start_lineno = tok.line;
            start_col_offset = tok.column;
        }
    }
    
    // Memoization helpers
    template<typename T>
    std::optional<std::pair<T, size_t>> get_memo(const void* rule_id) const {
        MemoKey key{position, rule_id};
        auto it = memo_cache.find(key);
        if (it != memo_cache.end() && it->second.success) {
            try {
                return std::make_pair(
                    std::any_cast<T>(it->second.value),
                    it->second.end_position
                );
            } catch (...) {
                return std::nullopt;
            }
        }
        return std::nullopt;
    }
    
    template<typename T>
    void set_memo(const void* rule_id, const T& value, size_t end_pos) {
        MemoKey key{position, rule_id};
        memo_cache[key] = MemoEntry{value, end_pos, true};
    }
    
    void set_memo_fail(const void* rule_id) {
        MemoKey key{position, rule_id};
        memo_cache[key] = MemoEntry{{}, position, false};
    }
};

// ============================================================================
// Template-based Rule Definitions
// ============================================================================

// Match a specific token type
template<TokenType Type>
struct MatchToken {
    static ParseResult<Token> parse(ParserState& state) {
        if (state.current().type == Type) {
            Token tok = state.current();
            state.advance();
            return tok;
        }
        return std::nullopt;
    }
};

// Match a keyword (single-quoted string in grammar)
template<TokenType Type>
using Keyword = MatchToken<Type>;

// Positive lookahead: &e (succeed if e parses, don't consume)
template<typename Parser>
struct Lookahead {
    static ParseResult<std::monostate> parse(ParserState& state) {
        size_t mark = state.mark();
        auto result = Parser::parse(state);
        state.reset(mark);  // Always restore position
        if (result) {
            return std::monostate{};
        }
        return std::nullopt;
    }
};

// Negative lookahead: !e (fail if e parses, don't consume)
template<typename Parser>
struct NegativeLookahead {
    static ParseResult<std::monostate> parse(ParserState& state) {
        size_t mark = state.mark();
        auto result = Parser::parse(state);
        state.reset(mark);  // Always restore position
        if (!result) {
            return std::monostate{};
        }
        return std::nullopt;
    }
};

// Optional: [e] or e?
template<typename Parser>
struct Optional {
    using value_type = std::optional<typename std::invoke_result_t<
        decltype(&Parser::parse), ParserState&>::value_type>;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        auto result = Parser::parse(state);
        if (result) {
            return value_type(*result);
        }
        state.reset(mark);
        return value_type(std::nullopt);
    }
};

// Zero or more: e*
template<typename Parser>
struct ZeroOrMore {
    using element_type = typename std::invoke_result_t<
        decltype(&Parser::parse), ParserState&>::value_type;
    using value_type = std::vector<element_type>;
    
    static ParseResult<value_type> parse(ParserState& state) {
        value_type results;
        while (true) {
            size_t mark = state.mark();
            auto result = Parser::parse(state);
            if (!result) {
                state.reset(mark);
                break;
            }
            results.push_back(std::move(*result));
        }
        return results;
    }
};

// One or more: e+
template<typename Parser>
struct OneOrMore {
    using element_type = typename std::invoke_result_t<
        decltype(&Parser::parse), ParserState&>::value_type;
    using value_type = std::vector<element_type>;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        auto first = Parser::parse(state);
        if (!first) {
            state.reset(mark);
            return std::nullopt;
        }
        
        value_type results;
        results.push_back(std::move(*first));
        
        while (true) {
            size_t inner_mark = state.mark();
            auto result = Parser::parse(state);
            if (!result) {
                state.reset(inner_mark);
                break;
            }
            results.push_back(std::move(*result));
        }
        return results;
    }
};

// Separated list: s.e+ (one or more e separated by s)
template<typename Separator, typename Parser>
struct SeparatedList {
    using element_type = typename std::invoke_result_t<
        decltype(&Parser::parse), ParserState&>::value_type;
    using value_type = std::vector<element_type>;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        auto first = Parser::parse(state);
        if (!first) {
            state.reset(mark);
            return std::nullopt;
        }
        
        value_type results;
        results.push_back(std::move(*first));
        
        while (true) {
            size_t inner_mark = state.mark();
            auto sep = Separator::parse(state);
            if (!sep) {
                state.reset(inner_mark);
                break;
            }
            auto elem = Parser::parse(state);
            if (!elem) {
                state.reset(inner_mark);
                break;
            }
            results.push_back(std::move(*elem));
        }
        return results;
    }
};

// Sequence: e1 e2 (parse e1 then e2)
template<typename... Parsers>
struct Sequence;

template<typename P1, typename P2>
struct Sequence<P1, P2> {
    using T1 = typename std::invoke_result_t<decltype(&P1::parse), ParserState&>::value_type;
    using T2 = typename std::invoke_result_t<decltype(&P2::parse), ParserState&>::value_type;
    using value_type = std::tuple<T1, T2>;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        
        auto r1 = P1::parse(state);
        if (!r1) {
            state.reset(mark);
            return std::nullopt;
        }
        
        auto r2 = P2::parse(state);
        if (!r2) {
            state.reset(mark);
            return std::nullopt;
        }
        
        return std::make_tuple(std::move(*r1), std::move(*r2));
    }
};

template<typename P1, typename P2, typename P3, typename... Rest>
struct Sequence<P1, P2, P3, Rest...> {
    using First = Sequence<P1, P2>;
    using RestSeq = Sequence<P3, Rest...>;
    
    static auto parse(ParserState& state) {
        size_t mark = state.mark();
        
        auto first = First::parse(state);
        if (!first) {
            state.reset(mark);
            return decltype(combine_results(first, RestSeq::parse(state))){};
        }
        
        auto rest = RestSeq::parse(state);
        if (!rest) {
            state.reset(mark);
            return decltype(combine_results(first, rest)){};
        }
        
        return combine_results(first, rest);
    }
    
private:
    template<typename T1, typename T2>
    static auto combine_results(ParseResult<T1>& r1, ParseResult<T2>& r2) {
        if (r1 && r2) {
            return ParseResult(std::tuple_cat(*r1, *r2));
        }
        return ParseResult<decltype(std::tuple_cat(*r1, *r2))>();
    }
};

// Choice: e1 | e2 (try e1, if fails try e2)
template<typename... Parsers>
struct Choice;

template<typename P1, typename P2>
struct Choice<P1, P2> {
    using T1 = typename std::invoke_result_t<decltype(&P1::parse), ParserState&>::value_type;
    using T2 = typename std::invoke_result_t<decltype(&P2::parse), ParserState&>::value_type;
    
    // If types are the same, use that type; otherwise use variant
    using value_type = std::conditional_t<
        std::is_same_v<T1, T2>,
        T1,
        std::variant<T1, T2>
    >;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        
        auto r1 = P1::parse(state);
        if (r1) {
            if constexpr (std::is_same_v<T1, T2>) {
                return *r1;
            } else {
                return value_type(std::in_place_index<0>, std::move(*r1));
            }
        }
        
        // Check if committed (cut operator)
        if (r1.committed) {
            return std::nullopt;  // Don't try alternatives
        }
        
        state.reset(mark);
        auto r2 = P2::parse(state);
        if (r2) {
            if constexpr (std::is_same_v<T1, T2>) {
                return *r2;
            } else {
                return value_type(std::in_place_index<1>, std::move(*r2));
            }
        }
        
        state.reset(mark);
        return std::nullopt;
    }
};

template<typename P1, typename P2, typename P3, typename... Rest>
struct Choice<P1, P2, P3, Rest...> {
    using First = P1;
    using RestChoice = Choice<P2, P3, Rest...>;
    
    using T1 = typename std::invoke_result_t<decltype(&First::parse), ParserState&>::value_type;
    using TRest = typename RestChoice::value_type;
    
    using value_type = std::conditional_t<
        std::is_same_v<T1, TRest>,
        T1,
        std::variant<T1, TRest>
    >;
    
    static ParseResult<value_type> parse(ParserState& state) {
        size_t mark = state.mark();
        
        auto r1 = First::parse(state);
        if (r1) {
            if constexpr (std::is_same_v<T1, TRest>) {
                return *r1;
            } else {
                return value_type(std::in_place_index<0>, std::move(*r1));
            }
        }
        
        if (r1.committed) {
            return std::nullopt;
        }
        
        state.reset(mark);
        auto r2 = RestChoice::parse(state);
        if (r2) {
            if constexpr (std::is_same_v<T1, TRest>) {
                return *r2;
            } else {
                return value_type(std::in_place_index<1>, std::move(*r2));
            }
        }
        
        state.reset(mark);
        return std::nullopt;
    }
};

// Cut operator: ~ (commit to current alternative)
struct Cut {
    static ParseResult<std::monostate> parse(ParserState& state) {
        ParseResult<std::monostate> result(std::monostate{});
        result.commit();
        return result;
    }
};

// ============================================================================
// Memoized Rule Wrapper
// ============================================================================

template<typename Parser, auto RuleId>
struct Memoized {
    using value_type = typename std::invoke_result_t<
        decltype(&Parser::parse), ParserState&>::value_type;
    
    static ParseResult<value_type> parse(ParserState& state) {
        // Check memo cache
        auto cached = state.get_memo<value_type>(reinterpret_cast<const void*>(RuleId));
        if (cached) {
            state.position = cached->second;
            return cached->first;
        }
        
        size_t start = state.mark();
        auto result = Parser::parse(state);
        
        if (result) {
            state.set_memo(reinterpret_cast<const void*>(RuleId), *result, state.mark());
        } else {
            state.set_memo_fail(reinterpret_cast<const void*>(RuleId));
        }
        
        return result;
    }
};

// ============================================================================
// AST Building Helpers
// ============================================================================

// Helper to create AST nodes with location info
template<typename NodeType, typename... Args>
std::shared_ptr<NodeType> make_ast(ParserState& state, Args&&... args) {
    return std::make_shared<NodeType>(
        std::forward<Args>(args)...,
        state.start_lineno,
        state.start_col_offset
    );
}

// EXTRA macro equivalent - provides location info
#define EXTRA state.start_lineno, state.start_col_offset

} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_PEG_PARSER_HPP
