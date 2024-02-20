#pragma once
#include "ast.h"
#include "scanner.h"

class Parser;

enum class Precedence {
    None,
    Assignment,     // =
    Boolean,        // and
    Equality,       // == !=
    Comparison,     // < > <= >=
    Term,           // + -
    Factor,         // * /
    Unary,          // ! -
    Call,           // . ()
    Primary
};

using ParseFunc = Expr (Parser::*)(bool);

struct ParseRule {
    ParseFunc prefix;
    ParseFunc infix;
    Precedence precedence;
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

    ParseRule getRule(TokenType type);

    Expr number(bool isLvalue);
    Expr identifier(bool isLvalue);
    Expr string(bool isLvalue);
    Expr literal(bool isLvalue);
    Expr grouping(bool isLvalue);
    Expr unary(bool isLvalue);
    Expr binary(bool isLvalue);
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