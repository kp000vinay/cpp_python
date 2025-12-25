#ifndef CPYTHON_CPP_TOKENIZER_V2_HPP
#define CPYTHON_CPP_TOKENIZER_V2_HPP

/**
 * Optimized Template-based Tokenizer
 * 
 * Key optimizations:
 * - Compile-time keyword lookup using constexpr
 * - Zero-copy string views where possible
 * - Type-safe token handling with templates
 * - Efficient indentation tracking
 * 
 * Reference: CPython's Parser/tokenizer.c, Grammar/Tokens
 */

#include <string>
#include <string_view>
#include <vector>
#include <array>
#include <optional>
#include <algorithm>
#include <cctype>
#include <cstdint>

namespace cpython_cpp {
namespace parser {

// ============================================================================
// Token Types - Matches CPython's Grammar/Tokens
// ============================================================================

enum class TokenType : uint8_t {
    // End markers
    ENDMARKER = 0,
    END_OF_FILE = ENDMARKER,
    
    // Newlines and indentation
    NEWLINE,
    INDENT,
    DEDENT,
    
    // Names and literals
    NAME,
    IDENTIFIER = NAME,
    NUMBER,
    STRING,
    
    // Keywords (alphabetically ordered for binary search)
    AND, AS, ASSERT, ASYNC, AWAIT,
    BREAK,
    CASE, CLASS, CONTINUE,
    DEF, DEL,
    ELIF, ELSE, EXCEPT,
    FALSE, FINALLY, FOR, FROM,
    GLOBAL,
    IF, IMPORT, IN, IS,
    LAMBDA,
    MATCH,
    NONE, NONLOCAL, NOT,
    OR,
    PASS,
    RAISE, RETURN,
    TRUE, TRY,
    WHILE, WITH,
    YIELD,
    
    // Operators
    PLUS,           // +
    MINUS,          // -
    STAR,           // *
    SLASH,          // /
    PERCENT,        // %
    DOUBLESTAR,     // **
    POWER = DOUBLESTAR,
    DOUBLESLASH,    // //
    FLOOR_DIV = DOUBLESLASH,
    AT,             // @
    
    // Comparison
    LESS,           // <
    GREATER,        // >
    EQEQUAL,        // ==
    EQUAL_EQUAL = EQEQUAL,
    NOTEQUAL,       // !=
    NOT_EQUAL = NOTEQUAL,
    LESSEQUAL,      // <=
    LESS_EQUAL = LESSEQUAL,
    GREATEREQUAL,   // >=
    GREATER_EQUAL = GREATEREQUAL,
    
    // Bitwise
    AMPER,          // &
    BIT_AND = AMPER,
    VBAR,           // |
    BIT_OR = VBAR,
    CIRCUMFLEX,     // ^
    BIT_XOR = CIRCUMFLEX,
    TILDE,          // ~
    BIT_NOT = TILDE,
    LEFTSHIFT,      // <<
    LEFT_SHIFT = LEFTSHIFT,
    RIGHTSHIFT,     // >>
    RIGHT_SHIFT = RIGHTSHIFT,
    
    // Delimiters
    LPAR,           // (
    LPAREN = LPAR,
    RPAR,           // )
    RPAREN = RPAR,
    LSQB,           // [
    LBRACKET = LSQB,
    RSQB,           // ]
    RBRACKET = RSQB,
    LBRACE,         // {
    RBRACE,         // }
    COLON,          // :
    COMMA,          // ,
    SEMI,           // ;
    SEMICOLON = SEMI,
    DOT,            // .
    ELLIPSIS,       // ...
    ARROW,          // ->
    COLONEQUAL,     // :=
    WALRUS = COLONEQUAL,
    EQUAL,          // =
    
    // Augmented assignment
    PLUSEQUAL,      // +=
    MINEQUAL,       // -=
    STAREQUAL,      // *=
    SLASHEQUAL,     // /=
    PERCENTEQUAL,   // %=
    DOUBLESTAREQUAL,// **=
    DOUBLESLASHEQUAL,// //=
    AMPEREQUAL,     // &=
    VBAREQUAL,      // |=
    CIRCUMFLEXEQUAL,// ^=
    LEFTSHIFTEQUAL, // <<=
    RIGHTSHIFTEQUAL,// >>=
    ATEQUAL,        // @=
    
    // F-string tokens
    FSTRING_START,
    FSTRING_MIDDLE,
    FSTRING_END,
    
    // Special
    EXCLAIM,        // ! (for f-string conversions)
    
    // Error token
    ERRORTOKEN,
    
    // Number of token types
    TOKEN_COUNT
};

// ============================================================================
// Token Structure
// ============================================================================

struct Token {
    TokenType type;
    std::string value;
    size_t line;
    size_t column;
    size_t start_offset;  // Byte offset in source
    size_t end_offset;
    
    Token() : type(TokenType::ENDMARKER), line(1), column(1), 
              start_offset(0), end_offset(0) {}
    
    Token(TokenType t, std::string v, size_t l, size_t c, 
          size_t start = 0, size_t end = 0)
        : type(t), value(std::move(v)), line(l), column(c),
          start_offset(start), end_offset(end) {}
    
    // Template-based type checking
    template<TokenType T>
    constexpr bool is() const noexcept { return type == T; }
    
    // Check if token is a keyword
    constexpr bool is_keyword() const noexcept {
        return type >= TokenType::AND && type <= TokenType::YIELD;
    }
    
    // Check if token is an operator
    constexpr bool is_operator() const noexcept {
        return type >= TokenType::PLUS && type <= TokenType::ATEQUAL;
    }
};

// ============================================================================
// Compile-time Keyword Lookup
// ============================================================================

namespace detail {

struct KeywordEntry {
    std::string_view name;
    TokenType type;
    
    constexpr KeywordEntry() : name(""), type(TokenType::IDENTIFIER) {}
    constexpr KeywordEntry(std::string_view n, TokenType t) : name(n), type(t) {}
};

// Sorted keyword table for binary search
constexpr std::array<KeywordEntry, 37> KEYWORDS = {{
    {"False", TokenType::FALSE},
    {"None", TokenType::NONE},
    {"True", TokenType::TRUE},
    {"and", TokenType::AND},
    {"as", TokenType::AS},
    {"assert", TokenType::ASSERT},
    {"async", TokenType::ASYNC},
    {"await", TokenType::AWAIT},
    {"break", TokenType::BREAK},
    {"case", TokenType::CASE},
    {"class", TokenType::CLASS},
    {"continue", TokenType::CONTINUE},
    {"def", TokenType::DEF},
    {"del", TokenType::DEL},
    {"elif", TokenType::ELIF},
    {"else", TokenType::ELSE},
    {"except", TokenType::EXCEPT},
    {"finally", TokenType::FINALLY},
    {"for", TokenType::FOR},
    {"from", TokenType::FROM},
    {"global", TokenType::GLOBAL},
    {"if", TokenType::IF},
    {"import", TokenType::IMPORT},
    {"in", TokenType::IN},
    {"is", TokenType::IS},
    {"lambda", TokenType::LAMBDA},
    {"match", TokenType::MATCH},
    {"nonlocal", TokenType::NONLOCAL},
    {"not", TokenType::NOT},
    {"or", TokenType::OR},
    {"pass", TokenType::PASS},
    {"raise", TokenType::RAISE},
    {"return", TokenType::RETURN},
    {"try", TokenType::TRY},
    {"while", TokenType::WHILE},
    {"with", TokenType::WITH},
    {"yield", TokenType::YIELD},
}};

// Compile-time keyword lookup
constexpr TokenType lookup_keyword(std::string_view name) noexcept {
    // Binary search
    size_t left = 0;
    size_t right = KEYWORDS.size();
    
    while (left < right) {
        size_t mid = left + (right - left) / 2;
        int cmp = name.compare(KEYWORDS[mid].name);
        
        if (cmp == 0) {
            return KEYWORDS[mid].type;
        } else if (cmp < 0) {
            right = mid;
        } else {
            left = mid + 1;
        }
    }
    
    return TokenType::IDENTIFIER;
}

} // namespace detail

// ============================================================================
// Tokenizer Class
// ============================================================================

class Tokenizer {
public:
    explicit Tokenizer(std::string source)
        : source_(std::move(source))
        , pos_(0)
        , line_(1)
        , column_(1)
        , at_line_start_(true)
        , paren_depth_(0) {
        indent_stack_.push_back(0);
    }
    
    // Get all tokens
    std::vector<Token> tokenize() {
        std::vector<Token> tokens;
        
        while (true) {
            Token tok = next_token();
            tokens.push_back(tok);
            
            if (tok.type == TokenType::ENDMARKER) {
                break;
            }
        }
        
        return tokens;
    }
    
    // Get next token
    Token next_token() {
        // Handle pending dedents
        if (!pending_dedents_.empty()) {
            Token tok = pending_dedents_.back();
            pending_dedents_.pop_back();
            return tok;
        }
        
        // Skip whitespace and comments (but track indentation at line start)
        if (at_line_start_ && paren_depth_ == 0) {
            return handle_indentation();
        }
        
        skip_whitespace_inline();
        skip_comment();
        
        // Check for end of file
        if (pos_ >= source_.size()) {
            // Generate remaining dedents
            while (indent_stack_.size() > 1) {
                indent_stack_.pop_back();
                pending_dedents_.push_back(
                    Token(TokenType::DEDENT, "", line_, column_));
            }
            if (!pending_dedents_.empty()) {
                Token tok = pending_dedents_.back();
                pending_dedents_.pop_back();
                return tok;
            }
            return Token(TokenType::ENDMARKER, "", line_, column_);
        }
        
        // Check for newline
        if (current() == '\n') {
            return read_newline();
        }
        
        // Check for string
        if (current() == '"' || current() == '\'') {
            return read_string();
        }
        
        // Check for f-string
        if ((current() == 'f' || current() == 'F' || 
             current() == 'r' || current() == 'R') &&
            (peek() == '"' || peek() == '\'')) {
            return read_fstring_or_string();
        }
        
        // Check for number
        if (std::isdigit(current()) || 
            (current() == '.' && std::isdigit(peek()))) {
            return read_number();
        }
        
        // Check for identifier/keyword
        if (std::isalpha(current()) || current() == '_') {
            return read_identifier();
        }
        
        // Check for operators and delimiters
        return read_operator();
    }

private:
    std::string source_;
    size_t pos_;
    size_t line_;
    size_t column_;
    bool at_line_start_;
    int paren_depth_;
    std::vector<size_t> indent_stack_;
    std::vector<Token> pending_dedents_;
    
    // F-string state
    struct FStringState {
        char quote;
        int quote_count;
        bool raw;
        int brace_depth;
    };
    std::vector<FStringState> fstring_stack_;
    
    // Character access
    char current() const noexcept {
        return pos_ < source_.size() ? source_[pos_] : '\0';
    }
    
    char peek(size_t offset = 1) const noexcept {
        size_t idx = pos_ + offset;
        return idx < source_.size() ? source_[idx] : '\0';
    }
    
    void advance() noexcept {
        if (pos_ < source_.size()) {
            if (source_[pos_] == '\n') {
                line_++;
                column_ = 1;
            } else {
                column_++;
            }
            pos_++;
        }
    }
    
    // Skip inline whitespace (not newlines)
    void skip_whitespace_inline() {
        while (pos_ < source_.size()) {
            char c = current();
            if (c == ' ' || c == '\t' || c == '\r') {
                advance();
            } else if (c == '\\' && peek() == '\n') {
                // Line continuation
                advance();
                advance();
            } else {
                break;
            }
        }
    }
    
    // Skip comment
    void skip_comment() {
        if (current() == '#') {
            while (pos_ < source_.size() && current() != '\n') {
                advance();
            }
        }
    }
    
    // Handle indentation at start of line
    Token handle_indentation() {
        size_t indent = 0;
        size_t start_line = line_;
        size_t start_col = column_;
        
        // Count leading whitespace
        while (pos_ < source_.size()) {
            char c = current();
            if (c == ' ') {
                indent++;
                advance();
            } else if (c == '\t') {
                // Tab = 8 spaces (CPython default)
                indent = (indent / 8 + 1) * 8;
                advance();
            } else if (c == '\n') {
                // Blank line - skip
                advance();
                indent = 0;
                start_line = line_;
                start_col = column_;
            } else if (c == '#') {
                // Comment line - skip to newline
                skip_comment();
                if (current() == '\n') {
                    advance();
                    indent = 0;
                    start_line = line_;
                    start_col = column_;
                }
            } else {
                break;
            }
        }
        
        at_line_start_ = false;
        
        // Check for end of file
        if (pos_ >= source_.size()) {
            return next_token();
        }
        
        // Compare with current indentation
        size_t current_indent = indent_stack_.back();
        
        if (indent > current_indent) {
            indent_stack_.push_back(indent);
            return Token(TokenType::INDENT, "", start_line, start_col);
        } else if (indent < current_indent) {
            // Generate dedents
            while (indent_stack_.size() > 1 && indent < indent_stack_.back()) {
                indent_stack_.pop_back();
                pending_dedents_.push_back(
                    Token(TokenType::DEDENT, "", start_line, start_col));
            }
            
            if (!pending_dedents_.empty()) {
                Token tok = pending_dedents_.back();
                pending_dedents_.pop_back();
                return tok;
            }
        }
        
        // Same indentation - continue to next token
        return next_token();
    }
    
    // Read newline token
    Token read_newline() {
        size_t start_line = line_;
        size_t start_col = column_;
        advance();
        
        // Inside parentheses, newlines are ignored
        if (paren_depth_ > 0) {
            skip_whitespace_inline();
            skip_comment();
            return next_token();
        }
        
        at_line_start_ = true;
        return Token(TokenType::NEWLINE, "\n", start_line, start_col);
    }
    
    // Read string literal
    Token read_string() {
        size_t start = pos_;
        size_t start_line = line_;
        size_t start_col = column_;
        
        char quote = current();
        advance();
        
        // Check for triple quote
        bool triple = false;
        if (current() == quote && peek() == quote) {
            triple = true;
            advance();
            advance();
        }
        
        std::string value;
        bool escaped = false;
        
        while (pos_ < source_.size()) {
            char c = current();
            
            if (escaped) {
                value += c;
                escaped = false;
                advance();
            } else if (c == '\\') {
                escaped = true;
                advance();
            } else if (c == quote) {
                if (triple) {
                    if (peek() == quote && peek(2) == quote) {
                        advance();
                        advance();
                        advance();
                        break;
                    }
                    value += c;
                    advance();
                } else {
                    advance();
                    break;
                }
            } else if (c == '\n' && !triple) {
                // Unterminated string
                break;
            } else {
                value += c;
                advance();
            }
        }
        
        return Token(TokenType::STRING, value, start_line, start_col, start, pos_);
    }
    
    // Read f-string or regular string
    Token read_fstring_or_string() {
        bool is_raw = (current() == 'r' || current() == 'R');
        bool is_fstring = (current() == 'f' || current() == 'F');
        
        if (is_raw && (peek() == 'f' || peek() == 'F')) {
            advance();
            is_fstring = true;
        } else if (is_fstring && (peek() == 'r' || peek() == 'R')) {
            advance();
            is_raw = true;
        }
        
        if (!is_fstring) {
            advance();  // Skip prefix
            return read_string();
        }
        
        // F-string handling
        size_t start_line = line_;
        size_t start_col = column_;
        advance();  // Skip 'f' or 'F'
        
        char quote = current();
        advance();
        
        // Check for triple quote
        int quote_count = 1;
        if (current() == quote && peek() == quote) {
            quote_count = 3;
            advance();
            advance();
        }
        
        fstring_stack_.push_back({quote, quote_count, is_raw, 0});
        
        return Token(TokenType::FSTRING_START, "", start_line, start_col);
    }
    
    // Read number literal
    Token read_number() {
        size_t start = pos_;
        size_t start_line = line_;
        size_t start_col = column_;
        
        // Handle different number formats
        if (current() == '0') {
            advance();
            if (current() == 'x' || current() == 'X') {
                // Hexadecimal
                advance();
                while (std::isxdigit(current()) || current() == '_') {
                    advance();
                }
            } else if (current() == 'o' || current() == 'O') {
                // Octal
                advance();
                while ((current() >= '0' && current() <= '7') || current() == '_') {
                    advance();
                }
            } else if (current() == 'b' || current() == 'B') {
                // Binary
                advance();
                while (current() == '0' || current() == '1' || current() == '_') {
                    advance();
                }
            } else {
                // Decimal or float starting with 0
                while (std::isdigit(current()) || current() == '_') {
                    advance();
                }
            }
        } else {
            // Decimal integer part
            while (std::isdigit(current()) || current() == '_') {
                advance();
            }
        }
        
        // Decimal point
        if (current() == '.' && std::isdigit(peek())) {
            advance();
            while (std::isdigit(current()) || current() == '_') {
                advance();
            }
        }
        
        // Exponent
        if (current() == 'e' || current() == 'E') {
            advance();
            if (current() == '+' || current() == '-') {
                advance();
            }
            while (std::isdigit(current()) || current() == '_') {
                advance();
            }
        }
        
        // Complex number suffix
        if (current() == 'j' || current() == 'J') {
            advance();
        }
        
        std::string value = source_.substr(start, pos_ - start);
        return Token(TokenType::NUMBER, value, start_line, start_col, start, pos_);
    }
    
    // Read identifier or keyword
    Token read_identifier() {
        size_t start = pos_;
        size_t start_line = line_;
        size_t start_col = column_;
        
        while (std::isalnum(current()) || current() == '_') {
            advance();
        }
        
        std::string value = source_.substr(start, pos_ - start);
        TokenType type = detail::lookup_keyword(value);
        
        return Token(type, value, start_line, start_col, start, pos_);
    }
    
    // Read operator or delimiter
    Token read_operator() {
        size_t start_line = line_;
        size_t start_col = column_;
        char c = current();
        
        // Three-character operators
        if (c == '.' && peek() == '.' && peek(2) == '.') {
            advance(); advance(); advance();
            return Token(TokenType::ELLIPSIS, "...", start_line, start_col);
        }
        if (c == '*' && peek() == '*' && peek(2) == '=') {
            advance(); advance(); advance();
            return Token(TokenType::DOUBLESTAREQUAL, "**=", start_line, start_col);
        }
        if (c == '/' && peek() == '/' && peek(2) == '=') {
            advance(); advance(); advance();
            return Token(TokenType::DOUBLESLASHEQUAL, "//=", start_line, start_col);
        }
        if (c == '<' && peek() == '<' && peek(2) == '=') {
            advance(); advance(); advance();
            return Token(TokenType::LEFTSHIFTEQUAL, "<<=", start_line, start_col);
        }
        if (c == '>' && peek() == '>' && peek(2) == '=') {
            advance(); advance(); advance();
            return Token(TokenType::RIGHTSHIFTEQUAL, ">>=", start_line, start_col);
        }
        
        // Two-character operators
        char c2 = peek();
        if (c == '+' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::PLUSEQUAL, "+=", start_line, start_col);
        }
        if (c == '-' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::MINEQUAL, "-=", start_line, start_col);
        }
        if (c == '-' && c2 == '>') {
            advance(); advance();
            return Token(TokenType::ARROW, "->", start_line, start_col);
        }
        if (c == '*' && c2 == '*') {
            advance(); advance();
            return Token(TokenType::DOUBLESTAR, "**", start_line, start_col);
        }
        if (c == '*' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::STAREQUAL, "*=", start_line, start_col);
        }
        if (c == '/' && c2 == '/') {
            advance(); advance();
            return Token(TokenType::DOUBLESLASH, "//", start_line, start_col);
        }
        if (c == '/' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::SLASHEQUAL, "/=", start_line, start_col);
        }
        if (c == '%' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::PERCENTEQUAL, "%=", start_line, start_col);
        }
        if (c == '@' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::ATEQUAL, "@=", start_line, start_col);
        }
        if (c == '&' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::AMPEREQUAL, "&=", start_line, start_col);
        }
        if (c == '|' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::VBAREQUAL, "|=", start_line, start_col);
        }
        if (c == '^' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::CIRCUMFLEXEQUAL, "^=", start_line, start_col);
        }
        if (c == '<' && c2 == '<') {
            advance(); advance();
            return Token(TokenType::LEFTSHIFT, "<<", start_line, start_col);
        }
        if (c == '>' && c2 == '>') {
            advance(); advance();
            return Token(TokenType::RIGHTSHIFT, ">>", start_line, start_col);
        }
        if (c == '<' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::LESSEQUAL, "<=", start_line, start_col);
        }
        if (c == '>' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::GREATEREQUAL, ">=", start_line, start_col);
        }
        if (c == '=' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::EQEQUAL, "==", start_line, start_col);
        }
        if (c == '!' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::NOTEQUAL, "!=", start_line, start_col);
        }
        if (c == ':' && c2 == '=') {
            advance(); advance();
            return Token(TokenType::COLONEQUAL, ":=", start_line, start_col);
        }
        
        // Single-character operators
        advance();
        switch (c) {
            case '+': return Token(TokenType::PLUS, "+", start_line, start_col);
            case '-': return Token(TokenType::MINUS, "-", start_line, start_col);
            case '*': return Token(TokenType::STAR, "*", start_line, start_col);
            case '/': return Token(TokenType::SLASH, "/", start_line, start_col);
            case '%': return Token(TokenType::PERCENT, "%", start_line, start_col);
            case '@': return Token(TokenType::AT, "@", start_line, start_col);
            case '&': return Token(TokenType::AMPER, "&", start_line, start_col);
            case '|': return Token(TokenType::VBAR, "|", start_line, start_col);
            case '^': return Token(TokenType::CIRCUMFLEX, "^", start_line, start_col);
            case '~': return Token(TokenType::TILDE, "~", start_line, start_col);
            case '<': return Token(TokenType::LESS, "<", start_line, start_col);
            case '>': return Token(TokenType::GREATER, ">", start_line, start_col);
            case '=': return Token(TokenType::EQUAL, "=", start_line, start_col);
            case '.': return Token(TokenType::DOT, ".", start_line, start_col);
            case ',': return Token(TokenType::COMMA, ",", start_line, start_col);
            case ':': return Token(TokenType::COLON, ":", start_line, start_col);
            case ';': return Token(TokenType::SEMI, ";", start_line, start_col);
            case '!': return Token(TokenType::EXCLAIM, "!", start_line, start_col);
            case '(':
                paren_depth_++;
                return Token(TokenType::LPAR, "(", start_line, start_col);
            case ')':
                paren_depth_ = std::max(0, paren_depth_ - 1);
                return Token(TokenType::RPAR, ")", start_line, start_col);
            case '[':
                paren_depth_++;
                return Token(TokenType::LSQB, "[", start_line, start_col);
            case ']':
                paren_depth_ = std::max(0, paren_depth_ - 1);
                return Token(TokenType::RSQB, "]", start_line, start_col);
            case '{':
                paren_depth_++;
                return Token(TokenType::LBRACE, "{", start_line, start_col);
            case '}':
                paren_depth_ = std::max(0, paren_depth_ - 1);
                return Token(TokenType::RBRACE, "}", start_line, start_col);
            default:
                return Token(TokenType::ERRORTOKEN, std::string(1, c), start_line, start_col);
        }
    }
};

} // namespace parser
} // namespace cpython_cpp

#endif // CPYTHON_CPP_TOKENIZER_V2_HPP
