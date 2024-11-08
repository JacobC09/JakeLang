#pragma once
#include <sstream>
#include "token.h"

class Scanner {
public:
    Scanner(std::string& src);

    Token nextToken();

private:
    char advance();
    char peek();
    char peekNext();
    char isAtEnd();
    bool match(char expected);

    void skipWhiteSpace();
    Token makeToken(TokenType type);

    Token scanNumber();
    Token scanString();
    Token scanIdentifer();

    int line;
    char* start;
    char* current;
    char* lineStart;
    std::string& source;
};