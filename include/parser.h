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

    std::unique_ptr<Ast> parse();

private:
    void advance();
    void consume(TokenType type, std::string msg);
    bool check(TokenType type);
    bool match(TokenType type);

    void errorAt(Token& token, std::string msg);
    void error(std::string msg);

    bool hadUnhandledError();
    bool isFinished();

    void number();
    void variable();
    void string();
    void literal();
    void grouping();
    void unary();
    void binary();

    void parsePrecedence(Precedence precedence);
    void expression();

    void expressionStatement();
    void printStatement();
    void returnStatement();
    void ifStatement();
    void whileLoop();
    void forLoop();
    void statement();
    
    void varDeclaration(); 
    void funcDeclaration();
    void classDeclaration();
    void declaration();

private:
    bool hadError;
    Token curr;
    Token prev;
    Scanner scanner;
};