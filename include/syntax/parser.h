#pragma once
#include "ast.h"
#include "scanner.h"

class Parser {
public:
    Parser(std::string& source);

    Ast parse();
    bool failed();

private:
    void advance();
    void errorAt(Token& token, std::string msg);
    void consume(TokenType type, std::string msg);
    bool check(TokenType type);
    bool match(TokenType type);

    template <typename... Args>
    bool match(TokenType type, Args... args);
    bool isFinished();

    Expr expression();
    Expr assignment();
    Expr _or();
    Expr _and();
    Expr equality();
    Expr comparison();
    Expr term();
    Expr factor();
    Expr exponent();
    Expr unary();
    Expr post();
    Expr primary();

    NumLiteral number();
    StrLiteral string();
    Identifier identifer();
    Expr grouping();

    std::vector<Expr> exprList();
    std::vector<Stmt> block();

    Stmt statement();
    Stmt exprStmt();
    Stmt printStmt();
    Stmt ifStmt();
    Stmt loopBlock();
    Stmt whileLoop();
    Stmt forLoop();
    Stmt returnStmt();

    Stmt funcDeclaration();
    Stmt varDeclaration();

private:
    bool hadError;
    Token cur;
    Token prev;
    Scanner scanner;
};