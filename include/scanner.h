#pragma once
#include <sstream>

enum class TokenType {
    // Single Char
    LeftParen, RightParen,
    LeftBrace, RightBrace,
    Comma, Dot, Plus, Minus,
    Slash, Asterisk, Carret, Semicolon, Percent,

    // One or Two Char
    Bang, BangEqual,
    Equal, EqualEqual,
    Greater, GreaterEqual,
    Less, LessEqual, PlusEqual, 
    MinusEqual, AsteriskEqual, SlashEqual,
    CarretEqual,

    // Literals
    Identifier, String, Number, True, False, None,

    // Keywords
    Print, If, Else, Loop, While, For, In, Continue, Break, Return, Func, Var, Exit, And, Or,

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

    int line;
    char* start;
    char* current;
    std::string source;
};