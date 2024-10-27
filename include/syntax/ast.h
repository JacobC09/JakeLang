#pragma once
#include <vector>

#include "variant.h"

struct Empty {};
struct NoneLiteral {};
struct BreakStmt {};
struct ContinueStmt {};

struct NumLiteral {
    double value;
};

struct Identifier {
    std::string name;
};

struct BoolLiteral {
    bool value;
};

struct StrLiteral {
    std::string value;
};

struct ExitStmt {
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
    Ptr<struct BlockStmt> >;

struct Ast {
    std::vector<Stmt> body;
};

// Expressions

struct AssignmentExpr {
    Expr target;
    Expr expr;
};

struct BinaryExpr {
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

    Operation op;
    Expr left;
    Expr right;
};

struct UnaryExpr {
    enum Operation {
        Negative,
        Negate
    };

    Operation op;
    Expr expr;
};

struct CallExpr {
    Expr target;
    std::vector<Expr> args;
};

struct PropertyExpr {
    Expr expr;
    Identifier prop;
};

// Statements

struct ExprStmt {
    Expr expr;
};

struct PrintStmt {
    std::vector<Expr> exprs;
};

struct IfStmt {
    Expr condition;
    std::vector<Stmt> body;
    std::vector<Stmt> orelse;
};

struct LoopBlock {
    std::vector<Stmt> body;
};

struct WhileLoop {
    Expr condition;
    std::vector<Stmt> body;
};

struct ForLoop {
    Identifier target;
    Expr iterator;
    std::vector<Stmt> body;
};

struct ReturnStmt {
    Expr value;
};

struct BlockStmt {
    std::vector<Stmt> body;
};

struct FuncDeclaration {
    Identifier name;
    std::vector<Identifier> args;
    std::vector<Stmt> body;
};

struct VarDeclaration {
    Identifier target;
    Expr expr;
};
