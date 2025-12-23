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

    // Operators
    PLUS, MINUS, STAR, SLASH, PERCENT, POWER, FLOOR_DIV, EQUAL,
    LPAREN, RPAREN, LBRACKET, RBRACKET, LBRACE, RBRACE,
    COLON, COMMA, SEMICOLON, DOT,
    GREATER, LESS, GREATER_EQUAL, LESS_EQUAL, EQUAL_EQUAL, NOT_EQUAL,
    BIT_OR, BIT_AND, BIT_XOR, BIT_NOT, LEFT_SHIFT, RIGHT_SHIFT,

    // Literals
    NUMBER, STRING, IDENTIFIER, TRUE, FALSE, NONE,

    // Special
    NEWLINE, INDENT, DEDENT, END_OF_FILE, ENDMARKER, ELLIPSIS,

    // Decorator
    AT
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

    void skip_whitespace();
    void skip_comment();
    Token read_number();
    Token read_string();
    Token read_identifier_or_keyword();
    TokenType keyword_to_token(const std::string& word);
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
        {"True", TokenType::TRUE},
        {"False", TokenType::FALSE},
        {"None", TokenType::NONE},
    };
}

// Implementations
inline Tokenizer::Tokenizer(const std::string& source)
    : source_(source), position_(0), line_(1), column_(1) {}

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

inline Token Tokenizer::next_token() {
    skip_whitespace();
    skip_comment();

    if (position_ >= source_.length()) {
        return Token(TokenType::END_OF_FILE, "", line_, column_);
    }

    char c = source_[position_];
    size_t start_col = column_;

    // Numbers
    if (std::isdigit(c)) {
        return read_number();
    }

    // Strings
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
            return Token(TokenType::PLUS, "+", line_, start_col);
        case '-':
            position_++; column_++;
            return Token(TokenType::MINUS, "-", line_, start_col);
        case '*':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '*') {
                position_++; column_++;
                return Token(TokenType::POWER, "**", line_, start_col);
            }
            return Token(TokenType::STAR, "*", line_, start_col);
        case '/':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '/') {
                position_++; column_++;
                return Token(TokenType::FLOOR_DIV, "//", line_, start_col);
            }
            return Token(TokenType::SLASH, "/", line_, start_col);
        case '%':
            position_++; column_++;
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
                return Token(TokenType::LEFT_SHIFT, "<<", line_, start_col);
            }
            return Token(TokenType::LESS, "<", line_, start_col);
        case '!':
            position_++; column_++;
            if (position_ < source_.length() && source_[position_] == '=') {
                position_++; column_++;
                return Token(TokenType::NOT_EQUAL, "!=", line_, start_col);
            }
            return Token(TokenType::NOT, "!", line_, start_col);
        case '|':
            position_++; column_++;
            return Token(TokenType::BIT_OR, "|", line_, start_col);
        case '&':
            position_++; column_++;
            return Token(TokenType::BIT_AND, "&", line_, start_col);
        case '^':
            position_++; column_++;
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

