#include "scanner.h"
#include "debug.h"

Scanner::Scanner(std::string src) {
    source = src;
    line = 1;
    current = source.data();
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
        case '^': return makeToken(match('=') ? TokenType::CarretEqual : TokenType::Carret);
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

    Token token = makeToken(TokenType::Identifier);

    if (token.value.compare("true") == 0) {
        token.type = TokenType::True;
    } else if (token.value.compare("false") == 0) {
        token.type = TokenType::False;
    } else if (token.value.compare("none") == 0) {
        token.type = TokenType::None;
    } else if (token.value.compare("print") == 0) {
        token.type = TokenType::Print;
    } else if (token.value.compare("if") == 0) {
        token.type = TokenType::If;
    } else if (token.value.compare("else") == 0) {
        token.type = TokenType::Else;
    } else if (token.value.compare("loop") == 0) {
        token.type = TokenType::Loop;
    } else if (token.value.compare("while") == 0) {
        token.type = TokenType::While;
    } else if (token.value.compare("for") == 0) {
        token.type = TokenType::For;
    } else if (token.value.compare("in") == 0) {
        token.type = TokenType::In;
    } else if (token.value.compare("continue") == 0) {
        token.type = TokenType::Continue;
    } else if (token.value.compare("break") == 0) {
        token.type = TokenType::Break;
    } else if (token.value.compare("return") == 0) {
        token.type = TokenType::Return;
    } else if (token.value.compare("func") == 0) {
        token.type = TokenType::Func;
    } else if (token.value.compare("var") == 0) {
        token.type = TokenType::Var;
    } else if (token.value.compare("exit") == 0) {
        token.type = TokenType::Exit;
    }

    return token;
}
