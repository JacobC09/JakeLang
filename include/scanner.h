#pragma once
#include <sstream>

enum class TokenType {
    // Single Char
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Comma, Dot, Plus, Minus,
    Slash, Asterisk, Semicolon,

    // One or Two Char
    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual, PlusEqual, MinusEqual, SlashEqual, AsteriskEqual,

    // literals
    Identifier, String, Number, True, False, None,

    Error, EndOfFile
};


struct Token {
    TokenType type;
    std::string_view value;
    int line;
    int column;
};

class Scanner {
public:
    Scanner() = default;
    Scanner(std::string src);

    Token nextToken();

private:
    char advance();
    char peek();
    char peekNext();
    char isAtEnd();
    bool match(char expected);

    void skipWhiteSpace();

    Token scanNumber();
    Token scanString();
    Token scanIdentifer();
    Token makeToken(TokenType type);

private:
    int line;
    char* lineStart;
    char* start;
    char* current;
    std::string source;
};