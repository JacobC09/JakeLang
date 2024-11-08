#pragma once
#include <vector>
#include "variant.h"
#include "token.h"

struct AstNode {
    SourceView view;
};

struct Empty : AstNode {};

struct NoneLiteral : AstNode {};

struct NumLiteral : AstNode {
    double value;
};

struct Identifier : AstNode {
    std::string name;
};

struct BoolLiteral : AstNode {
    bool value;
};

struct StrLiteral : AstNode {
    std::string value;
};

struct BreakStmt : AstNode {};

struct ContinueStmt : AstNode {};

struct ExitStmt : AstNode {
    NumLiteral code;
};

using Expr = Variant<
    Empty,
    NumLiteral,
    BoolLiteral,
    StrLiteral,
    NoneLiteral,
    Identifier,
    Ptr<struct AssignmentExpr>,
    Ptr<struct BinaryExpr>,
    Ptr<struct UnaryExpr>,
    Ptr<struct CallExpr>,
    Ptr<struct PropertyExpr> >;

using Stmt = Variant<
    Empty,
    BreakStmt,
    ContinueStmt,
    ExitStmt,
    Ptr<struct ExprStmt>,
    Ptr<struct PrintStmt>,
    Ptr<struct IfStmt>,
    Ptr<struct LoopBlock>,
    Ptr<struct WhileLoop>,
    Ptr<struct ForLoop>,
    Ptr<struct ReturnStmt>,
    Ptr<struct FuncDeclaration>,
    Ptr<struct VarDeclaration>,
    Ptr<struct BlockStmt>,
    Ptr<struct TypeDeclaration> >;

struct Ast {
    std::string source;
    std::vector<Stmt> body;
};

// Expressions

struct AssignmentExpr : AstNode {
    Expr target;
    Expr expr;
};

struct BinaryExpr : AstNode{
    enum Operation {
        Add,
        Subtract,
        Modulous,
        Multiply,
        Divide,
        Exponent,
        GreaterThan,
        LessThan,
        GreaterThanOrEq,
        LessThanOrEq,
        Equal,
        NotEqual,
        And,
        Or
    };

    Token opToken;
    Operation op;
    Expr left;
    Expr right;
};

struct UnaryExpr : AstNode{
    enum Operation {
        Negative,
        Negate
    };

    Token opToken;
    Operation op;
    Expr expr;
};

struct CallExpr : AstNode {
    Expr target;
    std::vector<Expr> args;
};

struct PropertyExpr : AstNode {
    Expr expr;
    Identifier prop;
};

// Statements

struct ExprStmt : AstNode {
    Expr expr;
};

struct PrintStmt : AstNode {
    std::vector<Expr> exprs;
};

struct IfStmt : AstNode {
    Expr condition;
    std::vector<Stmt> body;
    std::vector<Stmt> orelse;
};

struct LoopBlock : AstNode {
    std::vector<Stmt> body;
};

struct WhileLoop : AstNode {
    Expr condition;
    std::vector<Stmt> body;
};

struct ForLoop : AstNode {
    Identifier target;
    Expr iterator;
    std::vector<Stmt> body;
};

struct ReturnStmt : AstNode {
    Expr value;
};

struct BlockStmt : AstNode {
    std::vector<Stmt> body;
};

struct TypeDeclaration : AstNode {
    Identifier name;
    std::vector<Identifier> parents;
    std::vector<Stmt> methods;
};

struct FuncDeclaration : AstNode {
    Identifier name;
    std::vector<Identifier> args;
    std::vector<Stmt> body;
};

struct VarDeclaration : AstNode {
    Identifier target;
    Expr expr;
};

SourceView getSourceView(Expr& expr);
SourceView getSourceView(Stmt& stmt);
