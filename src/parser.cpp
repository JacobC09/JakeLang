#include "parser.h"
#include "variant"
#include "util.h"

Parser::Parser(std::string source) : scanner(source) {
    hadError = false;
}

Ast Parser::parse() {
    Ast ast;

    advance();
    while (!isFinished()) {
        ast.body.push_back(statement());
    }

    return std::move(ast);  
}

void Parser::advance() {
    if (hadError) return;

    prev = cur;
    cur = scanner.nextToken();

    if (cur.type == TokenType::Error) {
        errorAt(cur, formatStr("Invalid Token: %s", std::string(cur.value)));
    }
}

void Parser::consume(TokenType type, std::string msg) {
    if (cur.type == type) {
        advance();
        return;
    }

    errorAt(cur, msg);
};

bool Parser::check(TokenType type) {
    return cur.type == type;
}

void Parser::errorAt(Token& token, std::string msg) {
    hadError = true;

    printf("Error at line %d:", token.line);
    printf("\t%s", msg.c_str());
}

bool Parser::isFinished() {
    return check(TokenType::EndOfFile) || hadError;
}

Expr Parser::parsePrecedence(Precedence precedence) {
    // Todo;
    return Expr{};
}

Expr Parser::expression() {
    return parsePrecedence(Precedence::Assignment);
}

Stmt Parser::expressionStatement() {
    Stmt stmt = ExprStmt {expression()};
    consume(TokenType::Semicolon, "Expected ';' after expression");
    return stmt;
}

Stmt Parser::statement() {
    switch (cur.type) {
        default:
            return expressionStatement();
    }
}
