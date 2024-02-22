#include "scanner.h"

Scanner::Scanner(std::string src) : source(src) {
    line = 0;
    start = current = source.data();
    lineStart = start;
}

Token Scanner::nextToken() {
    skipWhiteSpace();

    start = current;

    if (isAtEnd()) return makeToken(TokenType::EndOfFile);

    char c = advance();

    if (isdigit(c)) return scanNumber();
    if (isalpha(c) || c == '_') return scanIdentifer();
    if (c == '\"' || c == '\'') return scanString();

    switch (c) {
        case '(': return makeToken(TokenType::LeftParen);
        case ')': return makeToken(TokenType::RightParen);
        case '{': return makeToken(TokenType::LeftBrace);
        case '}': return makeToken(TokenType::RightBrace);
        case ',': return makeToken(TokenType::Comma);
        case ';': return makeToken(TokenType::Semicolon);
        case '+': return makeToken(match('=') ? TokenType::PlusEqual : TokenType::Plus);
        case '-': return makeToken(match('=') ? TokenType::MinusEqual : TokenType::Minus);
        case '/': return makeToken(match('=') ? TokenType::SlashEqual : TokenType::Slash);
        case '*': return makeToken(match('=') ? TokenType::AsteriskEqual : TokenType::Asterisk);
        case '!': return makeToken(match('=') ? TokenType::BangEqual : TokenType::Bang);
        case '=': return makeToken(match('=') ? TokenType::EqualEqual : TokenType::Equal);
        case '>': return makeToken(match('=') ? TokenType::GreaterEqual : TokenType::Greater);
        case '<': return makeToken(match('=') ? TokenType::LessEqual : TokenType::Less);
            
        case '.':
            if (isdigit(peek()))
                return scanNumber();
            return makeToken(TokenType::Dot);
        
        default:
            return makeToken(TokenType::Error);
    }
};


char Scanner::advance() {
    current++;
    return current[-1];
}

char Scanner::peek() {
    return *current;
}

char Scanner::peekNext() {
    if (isAtEnd()) return '\0';

    return current[1];
}

char Scanner::isAtEnd() {
    return *current == '\0';
}

bool Scanner::match(char expected) {
    if (isAtEnd()) return false;
    if (*current != expected) return false;

    current++;
    return true; 
}

void Scanner::skipWhiteSpace() {
    for (;;) {
        switch (peek()) {
            case '\r':
            case '\t':
            case ' ':
                advance();
                break;
            case '\n':
                line++;
                advance();
                break;
            
            case '#':
                while (peek() != '\n' && !isAtEnd()) advance();
                break;

            default:
                return;
        }
    }
}

Token Scanner::makeToken(TokenType type) {
    int tokenLength = current - start;

    return Token {type, std::string_view(start, tokenLength), line};
}

Token Scanner::scanNumber() {
    while (isdigit(peek())) advance();

    if (peek() == '.') {
        advance();
        while (isdigit(peek())) advance();
    }

    return makeToken(TokenType::Number);
}

Token Scanner::scanString() {
    char startingChar = current[-1];

    while (peek() != startingChar) {
        if (peek() == '\n' || isAtEnd()) {
            return makeToken(TokenType::Error);
        }

        advance();
    }
        

    advance();
    return makeToken(TokenType::String);
}

Token Scanner::scanIdentifer() {
    while (isalpha(peek()) || isdigit(peek()) || peek() == '_') advance();
    return makeToken(TokenType::Identifier);
}
