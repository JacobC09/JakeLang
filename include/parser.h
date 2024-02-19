#pragma once
#include "ast.h"
#include "scanner.h"

enum class Precedence {
    None,
    Assignment,  // =
    Or,          // or
    And,         // and
    Equality,    // == !=
    Comparison,  // < > <= >=
    Term,        // + -
    Factor,      // * /
    Unary,       // ! -
    Call,        // . ()
    Primary
};

class Parser {
public:
    Parser(std::string source);

    Ast parse();

private:
    void advance();
    void consume(TokenType type, std::string msg);
    bool check(TokenType type);
    bool match(TokenType type);

    void errorAt(Token& token, std::string msg);

    bool hadUnhandledError();
    bool isFinished();

    Expr number();
    Expr variable();
    Expr string();
    Expr literal();
    Expr grouping();
    Expr unary();
    Expr binary();
    Expr parsePrecedence(Precedence precedence);
    Expr expression();

    Stmt expressionStatement();
    Stmt printStatement();
    Stmt returnStatement();
    Stmt ifStatement();
    Stmt whileLoop();
    Stmt forLoop();
    Stmt varDeclaration(); 
    Stmt funcDeclaration();
    Stmt classDeclaration();
    Stmt statement();
    

private:
    bool hadError;
    Token cur;
    Token prev;
    Scanner scanner;
};