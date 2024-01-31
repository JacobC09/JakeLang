#include "print.h"
#include "debug.h"

void printToken(const Token& token) {
    static const char* names[] = {
        "LeftParen", "RightParen",
        "LeftBrace", "RightBrace",
        "Comma", "Dot", "Plus", "Minus",
        "Slash", "Asterisk", "Semicolon",

        "Bang", "BangEqual",
        "Equal", "EqualEqual",
        "Greater", "GreaterEqual",
        "Less", "LessEqual", "PlusEqual", "MinusEqual", "SlashEqual", "AsteriskEqual",

        "Identifier", "String", "Number",

        "Error", "EndOfFile"
    };

    if (token.value.length() > 0) {
        printf("Token{type=%s, value=\'%s\'}\n", names[(int) token.type], std::string(token.value).c_str());
    } else {
        print((int) token.type);
        printf("Token{type=%s}\n", names[(int) token.type]);
    }
}
