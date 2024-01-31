#include "environment.h"
#include "ast.h"

#include "scanner.h"
#include "print.h"

Environment::Environment() {};

Result Environment::run(std::string source) {
    Scanner s = Scanner(source);
    
    while (1) {
        Token t = s.nextToken();

        if (t.type == TokenType::EndOfFile || t.type == TokenType::Error) {
            break;
        }

        printToken(t);
    }

    return Result { Success };
};