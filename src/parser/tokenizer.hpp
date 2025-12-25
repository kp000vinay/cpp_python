#ifndef CPYTHON_CPP_TOKENIZER_HPP
#define CPYTHON_CPP_TOKENIZER_HPP

#include <string>
#include <vector>
#include <cstdint>
#include <cctype>
#include <sstream>
#include <unordered_map>
#include <optional>
#include <array>
#include <concepts>

namespace cpython_cpp {
namespace parser {

/**
 * Template-based tokenizer with compile-time optimizations
 * Reference: Grammar/Tokens, Parser/tokenizer/
 *
 * Uses C++ templates for:
 * - Compile-time token type checking
 * - Zero-cost abstractions
 * - Constexpr keyword lookup
 * - Type-safe token matching
 */
enum class TokenType {
    // Keywords
    DEF, IF, ELIF, ELSE, FOR, WHILE, RETURN, CLASS, IMPORT, FROM, AS,
    AND, OR, NOT, IN, IS, TRY, EXCEPT, FINALLY, RAISE, WITH, PASS,
    BREAK, CONTINUE, LAMBDA, DEL, ASSERT, GLOBAL, NONLOCAL, YIELD,
    ASYNC, AWAIT,  // Python 3.5+ async/await keywords
    MATCH, CASE,   // Python 3.10+ pattern matching keywords

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT, POWER, FLOOR_DIV, EQUAL,
    LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,
    COLON, COMMA, SEMICOLON, DOT, WALRUS,  // := operator (Python 3.8+)
    ARROW,  // -> operator for return annotations (Python 3.5+)
    GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, EQUAL_EQUAL, NOT_EQUAL,
    BIT_OR, BIT_AND, BIT_XOR, BIT_NOT, LEFT_SHIFT, RIGHT_SHIFT,

    // Augmented assignment operators
    PLUSEQUAL, MINEQUAL, STAREQUAL, SLASHEQUAL, PERCENTEQUAL,
    DOUBLESTAREQUAL, DOUBLESLASHEQUAL, AMPEREQUAL, VBAREQUAL,
    CIRCUMFLEXEQUAL, LEFTSHIFTEQUAL, RIGHTSHIFTEQUAL,

    // Literals
    NUMBER, STRING, IDENTIFIER, TRUE, FALSE, NONE,

    // Special
    NEWLINE, INDENT, DEDENT, END_OF_FILE, ENDMARKER, ELLIPSIS,

    // Decorator
    AT,

    // F-string tokens
    FSTRING_START, FSTRING_MIDDLE, FSTRING_END,
    // Special token for f-string conversion specifier
    EXCLAIM  // ! in f-strings (for !s, !r, !a)
};

// Compile-time token tag for type-safe operations
template<TokenType T>
struct TokenTag {
    static constexpr TokenType value = T;
};

// Type-safe token with compile-time type information
template<TokenType Type>
struct TypedToken {
    static constexpr TokenType type = Type;
    std::string value;
    size_t line;
    size_t column;

    TypedToken(std::string v, size_t l, size_t c)
        : value(std::move(v)), line(l), column(c) {}
};

// Runtime token with template-based type checking
struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;

    Token(TokenType t, const std::string& v, size_t l, size_t c)
        : type(t), value(v), line(l), column(c) {}

    // Template-based type checking
    template<TokenType T>
    bool is() const { return type == T; }

    // Type-safe conversion to typed token
    template<TokenType T>
    std::optional<TypedToken<T>> as() const {
        if (type == T) {
            return TypedToken<T>{value, line, column};
        }
        return std::nullopt;
    }
};

// Compile-time operator precedence using templates
template<TokenType T>
struct Precedence {
    static constexpr int value = []() constexpr {
        if constexpr (T == TokenType::OR) return 1;
        else if constexpr (T == TokenType::AND) return 2;
        else if constexpr (T == TokenType::NOT) return 3;
        else if constexpr (T == TokenType::EQUAL_EQUAL || T == TokenType::NOT_EQUAL ||
                           T == TokenType::LESS || T == TokenType::GREATER ||
                           T == TokenType::LESS_EQUAL || T == TokenType::GREATER_EQUAL) return 4;
        else if constexpr (T == TokenType::BIT_OR) return 5;
        else if constexpr (T == TokenType::BIT_XOR) return 6;
        else if constexpr (T == TokenType::BIT_AND) return 7;
        else if constexpr (T == TokenType::LEFT_SHIFT || T == TokenType::RIGHT_SHIFT) return 8;
        else if constexpr (T == TokenType::PLUS || T == TokenType::MINUS) return 9;
        else if constexpr (T == TokenType::STAR || T == TokenType::SLASH ||
                           T == TokenType::PERCENT || T == TokenType::FLOOR_DIV) return 10;
        else if constexpr (T == TokenType::POWER) return 11;
        else return 0;
    }();
};

template<TokenType T>
inline constexpr int precedence_v = Precedence<T>::value;

/**
 * Tokenizer - converts Python source code to tokens
 *
 * This module can be developed independently.
 * Reference: Parser/tokenizer/, Grammar/Tokens
 */
class Tokenizer {
public:
    explicit Tokenizer(const std::string& source);

    // Get next token
    Token next_token();

    // Check if more tokens available
    bool has_more() const;

    // Get all tokens (for testing)
    std::vector<Token> tokenize();

private:
    std::string source_;
    size_t position_;
    size_t line_;
    size_t column_;

    // F-string state tracking
    struct FStringState {
        char quote;           // ' or "
        int quote_size;       // 1 for single quote, 3 for triple quote
        bool raw;             // true for raw f-strings (rf or fr)
        int curly_brace_depth; // Track nesting depth for expressions
        int curly_brace_expr_start_depth; // Depth when current expression started (-1 if none)
        bool in_format_spec;  // Are we in format specifier mode (after : in expression)?
        bool has_format_spec; // Have we seen : at the top-level expression?
    };
    std::vector<FStringState> fstring_stack_;  // Stack for nested f-strings

    void skip_whitespace();
    void skip_comment();
    Token read_number();
    Token read_string();
    Token read_identifier_or_keyword();
    TokenType keyword_to_token(const std::string& word);

    // F-string lexing functions
    bool check_fstring_prefix(size_t& prefix_len, bool& is_raw);
    Token read_fstring_start();
    Token read_fstring_middle();
    Token read_fstring_end();
};

// Keyword map
namespace {
    const std::unordered_map<std::string, TokenType> keywords = {
        {"def", TokenType::DEF},
        {"if", TokenType::IF},
        {"elif", TokenType::ELIF},
        {"else", TokenType::ELSE},
        {"for", TokenType::FOR},
        {"while", TokenType::WHILE},
        {"return", TokenType::RETURN},
        {"class", TokenType::CLASS},
        {"import", TokenType::IMPORT},
        {"from", TokenType::FROM},
        {"as", TokenType::AS},
        {"and", TokenType::AND},
        {"or", TokenType::OR},
        {"not", TokenType::NOT},
        {"in", TokenType::IN},
        {"is", TokenType::IS},
        {"try", TokenType::TRY},
        {"except", TokenType::EXCEPT},
        {"finally", TokenType::FINALLY},
        {"raise", TokenType::RAISE},
        {"pass", TokenType::PASS},
        {"del", TokenType::DEL},
        {"assert", TokenType::ASSERT},
        {"global", TokenType::GLOBAL},
        {"nonlocal", TokenType::NONLOCAL},
        {"with", TokenType::WITH},
        {"break", TokenType::BREAK},
        {"continue", TokenType::CONTINUE},
        {"lambda", TokenType::LAMBDA},
        {"yield", TokenType::YIELD},
        {"async", TokenType::ASYNC},  // Python 3.5+
        {"await", TokenType::AWAIT},  // Python 3.5+
        {"match", TokenType::MATCH},  // Python 3.10+ soft keyword
        {"case", TokenType::CASE},    // Python 3.10+ soft keyword
        {"True", TokenType::TRUE},
        {"False", TokenType::FALSE},
        {"None", TokenType::NONE},
    };
}

// Implementations
inline Tokenizer::Tokenizer(const std::string& source)
    : source_(source), position_(0), line_(1), column_(1), fstring_stack_() {}

inline void Tokenizer::skip_whitespace() {
    while (position_ < source_.length()) {
        char c = source_[position_];
        if (c == ' ' || c == '\t') {
            position_++;
            column_++;
        } else if (c == '\n') {
            position_++;
            line_++;
            column_ = 1;
        } else {
            break;
        }
    }
}

inline void Tokenizer::skip_comment() {
    if (position_ < source_.length() && source_[position_] == '#') {
        while (position_ < source_.length() && source_[position_] != '\n') {
            position_++;
        }
    }
}

inline Token Tokenizer::read_number() {
    size_t start = position_;
    size_t start_col = column_;

    // Read integer part
    while (position_ < source_.length() && std::isdigit(source_[position_])) {
        position_++;
        column_++;
    }

    // Check for decimal point
    if (position_ < source_.length() && source_[position_] == '.') {
        position_++;
        column_++;
        while (position_ < source_.length() && std::isdigit(source_[position_])) {
            position_++;
            column_++;
        }
    }

    std::string value = source_.substr(start, position_ - start);
    return Token(TokenType::NUMBER, value, line_, start_col);
}

inline Token Tokenizer::read_string() {
    char quote = source_[position_];
    size_t start_col = column_;
    position_++;
    column_++;

    std::string value;
    bool escaped = false;

    while (position_ < source_.length()) {
        char c = source_[position_];

        if (escaped) {
            value += c;
            escaped = false;
            position_++;
            column_++;
        } else if (c == '\\') {
            escaped = true;
            position_++;
            column_++;
        } else if (c == quote) {
            position_++;
            column_++;
            break;
        } else if (c == '\n') {
            // Multi-line strings not fully handled here
            value += c;
            position_++;
            line_++;
            column_ = 1;
        } else {
            value += c;
            position_++;
            column_++;
        }
    }

    return Token(TokenType::STRING, value, line_, start_col);
}

inline Token Tokenizer::read_identifier_or_keyword() {
    size_t start = position_;
    size_t start_col = column_;

    while (position_ < source_.length() &&
           (std::isalnum(source_[position_]) || source_[position_] == '_')) {
        position_++;
        column_++;
    }

    std::string word = source_.substr(start, position_ - start);
    TokenType type = keyword_to_token(word);

    return Token(type, word, line_, start_col);
}

inline TokenType Tokenizer::keyword_to_token(const std::string& word) {
    auto it = keywords.find(word);
    if (it != keywords.end()) {
        return it->second;
    }
    return TokenType::IDENTIFIER;
}

// Check if we have an f-string prefix (f/F/rf/fr) before a quote
// Returns true if f-string detected, and sets is_raw and has_f
inline bool Tokenizer::check_fstring_prefix(size_t& prefix_len, bool& is_raw) {
    size_t saved_pos = position_;
    size_t saved_col = column_;
    prefix_len = 0;
    is_raw = false;
    bool has_f = false;

    // Check for r/R prefix (raw) - can come before or after f
    if (position_ < source_.length() &&
        (source_[position_] == 'r' || source_[position_] == 'R')) {
        is_raw = true;
        prefix_len++;
        position_++;
        column_++;
    }

    // Check for f/F prefix
    if (position_ < source_.length() &&
        (source_[position_] == 'f' || source_[position_] == 'F')) {
        has_f = true;
        prefix_len++;
        position_++;
        column_++;
    }

    // If we saw r but not f, check if f comes after
    if (is_raw && !has_f && position_ < source_.length() &&
        (source_[position_] == 'f' || source_[position_] == 'F')) {
        has_f = true;
        prefix_len++;
        position_++;
        column_++;
    }

    // Check if we have a quote after the prefix
    if (has_f && position_ < source_.length() &&
        (source_[position_] == '"' || source_[position_] == '\'')) {
        // Don't restore - we'll consume in read_fstring_start
        return true;
    }

    // No f-string, restore position
    position_ = saved_pos;
    column_ = saved_col;
    return false;
}

// Read f-string start token
inline Token Tokenizer::read_fstring_start() {
    size_t start_pos = position_;
    size_t start_col = column_;

    // Check prefix (already detected, but we need to determine raw)
    bool is_raw = false;
    bool has_f = false;

    // Check for r/R prefix (raw) - can come before or after f
    if (position_ < source_.length() &&
        (source_[position_] == 'r' || source_[position_] == 'R')) {
        is_raw = true;
        position_++;
        column_++;
    }

    // Check for f/F prefix
    if (position_ < source_.length() &&
        (source_[position_] == 'f' || source_[position_] == 'F')) {
        has_f = true;
        position_++;
        column_++;
    }

    // If we saw r but not f, check if f comes after
    if (is_raw && !has_f && position_ < source_.length() &&
        (source_[position_] == 'f' || source_[position_] == 'F')) {
        has_f = true;
        position_++;
        column_++;
    }

    if (!has_f) {
        // Should not happen if check_fstring_prefix worked correctly
        return Token(TokenType::ENDMARKER, "", line_, start_col);
    }

    // Determine quote and quote size
    char quote = source_[position_];
    int quote_size = 1;

    // Check for triple quotes
    if (position_ + 2 < source_.length() &&
        source_[position_] == source_[position_ + 1] &&
        source_[position_] == source_[position_ + 2]) {
        quote_size = 3;
        position_ += 3;
        column_ += 3;
    } else {
        position_++;
        column_++;
    }

    // Push f-string state
    FStringState state;
    state.quote = quote;
    state.quote_size = quote_size;
    state.raw = is_raw;
    state.curly_brace_depth = 0;
    state.curly_brace_expr_start_depth = -1;  // -1 means no active expression
    state.in_format_spec = false;
    state.has_format_spec = false;
    fstring_stack_.push_back(state);

    // Read initial string content until { or end quote
    std::string value;
    bool escaped = false;

    while (position_ < source_.length()) {
        char c = source_[position_];

        // Check for end quote (must check before other processing)
        if (!escaped && c == quote) {
            // Check if it's the closing quote (same size as opening)
            bool is_closing = true;
            for (int i = 0; i < quote_size; i++) {
                if (position_ + i >= source_.length() ||
                    source_[position_ + i] != quote) {
                    is_closing = false;
                    break;
                }
            }
            if (is_closing) {
                // This is the end - don't consume the quote, return FSTRING_START
                // The next token will be FSTRING_END
                break;
            }
        }

        // Check for { (start of expression) - only if not at end quote
        if (!escaped && c == '{') {
            // Check if it's {{ (literal brace)
            if (position_ + 1 < source_.length() && source_[position_ + 1] == '{') {
                value += '{';
                position_ += 2;
                column_ += 2;
                continue;
            } else {
                // Start of expression - return FSTRING_START and let parser handle it
                break;
            }
        }

        // Handle escape sequences
        if (escaped) {
            if (is_raw && c != quote && c != '\\') {
                // In raw f-strings, only quote and backslash can be escaped
                value += '\\';
            }
            value += c;
            escaped = false;
            position_++;
            column_++;
        } else if (c == '\\' && !is_raw) {
            escaped = true;
            position_++;
            column_++;
        } else {
            value += c;
            position_++;
            if (c == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
        }
    }

    return Token(TokenType::FSTRING_START, value, line_, start_col);
}

// Read f-string middle token (literal text between expressions)
inline Token Tokenizer::read_fstring_middle() {
    if (fstring_stack_.empty()) {
        return Token(TokenType::ENDMARKER, "", line_, column_);
    }

    FStringState& state = fstring_stack_.back();
    size_t start_col = column_;
    std::string value;
    bool escaped = false;

    while (position_ < source_.length()) {
        char c = source_[position_];

        // Check for end quote
        if (!escaped && c == state.quote) {
            bool is_closing = true;
            for (int i = 0; i < state.quote_size; i++) {
                if (position_ + i >= source_.length() ||
                    source_[position_ + i] != state.quote) {
                    is_closing = false;
                    break;
                }
            }
            if (is_closing) {
                // End of f-string
                break;
            }
        }

        // Check for { (start of expression)
        if (!escaped && c == '{') {
            if (position_ + 1 < source_.length() && source_[position_ + 1] == '{') {
                // Literal {{ - emit single {
                value += '{';
                position_ += 2;
                column_ += 2;
                continue;
            } else {
                // Start of expression
                break;
            }
        }

        // Check for } (end of expression or format spec)
        if (!escaped && c == '}') {
            if (position_ + 1 < source_.length() && source_[position_ + 1] == '}') {
                // Literal }} - emit single }
                value += '}';
                position_ += 2;
                column_ += 2;
                continue;
            } else {
                // End of expression or format spec - stop reading
                break;
            }
        }

        // Handle escape sequences
        if (escaped) {
            if (state.raw && c != state.quote && c != '\\') {
                value += '\\';
            }
            value += c;
            escaped = false;
            position_++;
            column_++;
        } else if (c == '\\' && !state.raw) {
            escaped = true;
            position_++;
            column_++;
        } else {
            value += c;
            position_++;
            if (c == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
        }
    }

    return Token(TokenType::FSTRING_MIDDLE, value, line_, start_col);
}

// Read f-string end token
inline Token Tokenizer::read_fstring_end() {
    if (fstring_stack_.empty()) {
        return Token(TokenType::ENDMARKER, "", line_, column_);
    }

    FStringState& state = fstring_stack_.back();
    size_t start_col = column_;

    // Consume closing quote
    for (int i = 0; i < state.quote_size; i++) {
        if (position_ < source_.length() && source_[position_] == state.quote) {
            position_++;
            column_++;
        }
    }

    // Pop f-string state
    fstring_stack_.pop_back();

    return Token(TokenType::FSTRING_END, "", line_, start_col);
}

inline Token Tokenizer::next_token() {
    skip_whitespace();
    skip_comment();

    if (position_ >= source_.length()) {
        return Token(TokenType::END_OF_FILE, "", line_, column_);
    }

    char c = source_[position_];
    size_t start_col = column_;

    // Check if we're in an f-string (MUST be before number check!)
    if (!fstring_stack_.empty()) {
        FStringState& state = fstring_stack_.back();

        // Check for closing quote
        if (c == state.quote) {
            bool is_closing = true;
            for (int i = 0; i < state.quote_size; i++) {
                if (position_ + i >= source_.length() ||
                    source_[position_ + i] != state.quote) {
                    is_closing = false;
                    break;
                }
            }
            if (is_closing) {
                return read_fstring_end();
            }
        }

        // Check for { - start of expression (unless {{)
        if (c == '{') {
            if (position_ + 1 < source_.length() && source_[position_ + 1] == '{') {
                // Literal {{ - handled in read_fstring_middle
                return read_fstring_middle();
            } else {
                // Start of expression - increment depth and emit LBRACE
                if (state.in_format_spec) {
                    // Nested expression in format spec - exit format spec mode
                    state.in_format_spec = false;
                }
                if (state.curly_brace_depth == 0) {
                    // This is the start of a new top-level expression
                    state.curly_brace_expr_start_depth = 0;
                }
                state.curly_brace_depth++;
                position_++;
                column_++;
                return Token(TokenType::LBRACE, "{", line_, start_col);
            }
        }

        // Check for } - end of expression (if depth matches)
        if (c == '}') {
            // Check for literal }} - only when NOT inside an expression (depth == 0)
            if (state.curly_brace_depth == 0 && 
                position_ + 1 < source_.length() && source_[position_ + 1] == '}') {
                // Literal }} - handled in read_fstring_middle
                return read_fstring_middle();
            } else if (state.curly_brace_depth > 0) {
                // End of expression - decrement depth and emit RBRACE
                state.curly_brace_depth--;
                if (state.curly_brace_depth == 0) {
                    // End of top-level expression
                    state.in_format_spec = false;
                    state.has_format_spec = false;
                    state.curly_brace_expr_start_depth = -1;
                } else if (state.has_format_spec && 
                           state.curly_brace_depth - 1 == state.curly_brace_expr_start_depth) {
                    // We're back at the format spec level after a nested expression
                    state.in_format_spec = true;
                }
                position_++;
                column_++;
                return Token(TokenType::RBRACE, "}", line_, start_col);
            } else {
                // This shouldn't happen - unmatched }
                position_++;
                column_++;
                return Token(TokenType::RBRACE, "}", line_, start_col);
            }
        }

        // Check for : at the expression start depth (format specifier)
        // This must be checked BEFORE falling through to normal tokenization
        if (c == ':' && state.curly_brace_depth > 0 && 
            state.curly_brace_depth - 1 == state.curly_brace_expr_start_depth) {
            // This is the format specifier colon
            state.in_format_spec = true;
            state.has_format_spec = true;  // Remember we've seen :
            position_++;
            column_++;
            return Token(TokenType::COLON, ":", line_, start_col);
        }

        // If we're inside an expression (depth > 0) and NOT in format spec, tokenize normally
        // Otherwise, read f-string middle (literal text)
        if (state.curly_brace_depth > 0 && !state.in_format_spec) {
            // We're inside an expression - fall through to normal tokenization
        } else {
            return read_fstring_middle();
        }
    }

    // Numbers (checked after f-string to allow format specs like "0>5")
    if (std::isdigit(c)) {
        return read_number();
    }

    // Check for f-string prefix before quote
    // Peek ahead to see if we have f/F/rf/fr before a quote
    if ((c == 'f' || c == 'F' || c == 'r' || c == 'R') && position_ + 1 < source_.length()) {
        size_t peek_pos = position_;
        bool is_raw = false;
        bool has_f = false;

        // Check for r/R
        if (source_[peek_pos] == 'r' || source_[peek_pos] == 'R') {
            is_raw = true;
            peek_pos++;
        }

        // Check for f/F
        if (peek_pos < source_.length() &&
            (source_[peek_pos] == 'f' || source_[peek_pos] == 'F')) {
            has_f = true;
            peek_pos++;
        } else if (is_raw && peek_pos < source_.length() &&
                   (source_[peek_pos] == 'f' || source_[peek_pos] == 'F')) {
            has_f = true;
            peek_pos++;
        }

        // Check if we have a quote after the prefix
        if (has_f && peek_pos < source_.length() &&
            (source_[peek_pos] == '"' || source_[peek_pos] == '\'')) {
            // We have an f-string - read the start
            return read_fstring_start();
        }
    }

    // Strings (non-f-string)
    if (c == '"' || c == '\'') {
        return read_string();
    }

    // Identifiers and keywords
    if (std::isalpha(c) || c == '_') {
        return read_identifier_or_keyword();
    }

    // Operators and punctuation
    switch (c) {
        case '+':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::PLUSEQUAL, "+=", line_, start_col);
            }
            return Token(TokenType::PLUS, "+", line_, start_col);
        case '-':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::MINEQUAL, "-=", line_, start_col);
            }
            // Check for arrow operator ->
            if (position_ < source_.length() && source_[position_] == '>') {
                position_++; column_++;
                return Token(TokenType::ARROW, "->", line_, start_col);
            }
            return Token(TokenType::MINUS, "-", line_, start_col);
        case '*':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '*') {
                position_++; column_++;
                // Check for **=
                if (position_ < source_.length() && source_[position_] == '=') {
                    position_++; column_++;
                    return Token(TokenType::DOUBLESTAREQUAL, "**=", line_, start_col);
                }
                return Token(TokenType::POWER, "**", line_, start_col);
            }
            // Check for *=
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::STAREQUAL, "*=", line_, start_col);
            }
            return Token(TokenType::STAR, "*", line_, start_col);
        case '/':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '/') {
                position_++; column_++;
                // Check for //=
                if (position_ < source_.length() && source_[position_] == '=') {
                    position_++; column_++;
                    return Token(TokenType::DOUBLESLASHEQUAL, "//=", line_, start_col);
                }
                return Token(TokenType::FLOOR_DIV, "//", line_, start_col);
            }
            // Check for /=
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::SLASHEQUAL, "/=", line_, start_col);
            }
            return Token(TokenType::SLASH, "/", line_, start_col);
        case '%':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::PERCENTEQUAL, "%=", line_, start_col);
            }
            return Token(TokenType::PERCENT, "%", line_, start_col);
        case '=':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::EQUAL_EQUAL, "==", line_, start_col);
            }
            return Token(TokenType::EQUAL, "=", line_, start_col);
        case '>':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::GREATER_EQUAL, ">=", line_, start_col);
            } else if (position_ < source_.length() && source_[position_] == '>') {
                position_++; column_++;
                // Check for >>=
                if (position_ < source_.length() && source_[position_] == '=') {
                    position_++; column_++;
                    return Token(TokenType::RIGHTSHIFTEQUAL, ">>=", line_, start_col);
                }
                return Token(TokenType::RIGHT_SHIFT, ">>", line_, start_col);
            }
            return Token(TokenType::GREATER, ">", line_, start_col);
        case '<':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::LESS_EQUAL, "<=", line_, start_col);
            } else if (position_ < source_.length() && source_[position_] == '<') {
                position_++; column_++;
                // Check for <<=
                if (position_ < source_.length() && source_[position_] == '=') {
                    position_++; column_++;
                    return Token(TokenType::LEFTSHIFTEQUAL, "<<=", line_, start_col);
                }
                return Token(TokenType::LEFT_SHIFT, "<<", line_, start_col);
            }
            return Token(TokenType::LESS, "<", line_, start_col);
        case '!':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::NOT_EQUAL, "!=", line_, start_col);
            }
            // In f-string expression context, ! is for conversion specifiers (!s, !r, !a)
            if (!fstring_stack_.empty() && fstring_stack_.back().curly_brace_depth > 0) {
                return Token(TokenType::EXCLAIM, "!", line_, start_col);
            }
            // Standalone ! is not valid in Python (only != is valid)
            // But we'll tokenize it as NOT for error reporting
            return Token(TokenType::NOT, "!", line_, start_col);
        case '|':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::VBAREQUAL, "|=", line_, start_col);
            }
            return Token(TokenType::BIT_OR, "|", line_, start_col);
        case '&':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::AMPEREQUAL, "&=", line_, start_col);
            }
            return Token(TokenType::BIT_AND, "&", line_, start_col);
        case '^':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::CIRCUMFLEXEQUAL, "^=", line_, start_col);
            }
            return Token(TokenType::BIT_XOR, "^", line_, start_col);
        case '~':
            position_++; column_++;
            return Token(TokenType::BIT_NOT, "~", line_, start_col);
        case '(':
            position_++; column_++;
            return Token(TokenType::LPAREN, "(", line_, start_col);
        case ')':
            position_++; column_++;
            return Token(TokenType::RPAREN, ")", line_, start_col);
        case '[':
            position_++; column_++;
            return Token(TokenType::LBRACKET, "[", line_, start_col);
        case ']':
            position_++; column_++;
            return Token(TokenType::RBRACKET, "]", line_, start_col);
        case '{':
            position_++; column_++;
            return Token(TokenType::LBRACE, "{", line_, start_col);
        case '}':
            position_++; column_++;
            return Token(TokenType::RBRACE, "}", line_, start_col);
        case ':':
            position_++; column_++;
            // Check for walrus operator :=
            if (position_ < source_.size() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::WALRUS, ":=", line_, start_col);
            }
            return Token(TokenType::COLON, ":", line_, start_col);
        case ',':
            position_++; column_++;
            return Token(TokenType::COMMA, ",", line_, start_col);
        case ';':
            position_++; column_++;
            return Token(TokenType::SEMICOLON, ";", line_, start_col);
        case '.':
            position_++; column_++;
            // Check for ellipsis: ...
            if (position_ < source_.length() && source_[position_] == '.') {
                position_++; column_++;
                if (position_ < source_.length() && source_[position_] == '.') {
                    position_++; column_++;
                    return Token(TokenType::ELLIPSIS, "...", line_, start_col);
                }
                // Two dots - this is invalid, but we'll treat as two DOT tokens
                // Reset to handle as two separate dots
                position_--; column_--;
                return Token(TokenType::DOT, ".", line_, start_col);
            }
            return Token(TokenType::DOT, ".", line_, start_col);
        case '@':
            position_++; column_++;
            return Token(TokenType::AT, "@", line_, start_col);
        case '\n':
            position_++; line_++; column_ = 1;
            return Token(TokenType::NEWLINE, "\n", line_ - 1, start_col);
        default:
            position_++; column_++;
            return Token(TokenType::ENDMARKER, std::string(1, c), line_, start_col);
    }
}

inline bool Tokenizer::has_more() const {
    return position_ < source_.length();
}

inline std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;
    Token token = next_token();

    while (token.type != TokenType::END_OF_FILE) {
        tokens.push_back(token);
        token = next_token();
    }
    tokens.push_back(token); // Add END_OF_FILE token

    return tokens;
}

} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_TOKENIZER_HPP

