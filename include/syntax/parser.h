#pragma once
#include "ast.h"
#include "scanner.h"
#include "error.h"

class Parser {
public:
    Parser(std::string& src, std::string& path);

    Ast parse();
    bool failed();
    Error getError();

private:
    void advance();
    void errorAt(Token& token, std::string msg, std::string note="");
    void errorAtView(SourceView view, std::string msg, std::string note="");
    void consume(TokenType type, std::string msg);
    bool isFinished();
    bool check(TokenType type);
    bool match(TokenType type);

    template <typename... Args>
    bool match(TokenType type, Args... args);

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

    Stmt typeDeclaration();
    Stmt funcDeclaration();
    Stmt methodDeclaration();
    Stmt varDeclaration();

private:
    bool hadError;
    Error error;
    Token cur;
    Token prev;
    Scanner scanner;
    std::string& source;
    std::string& path;
};