#pragma once
#include <vector>
#include "variant.h"

struct Empty {};

// Literals

struct NumLiteral {
    double value;
};

struct Identifier {
    std::string value;
};

struct BoolLiteral {
    bool value;
};

struct StrLiteral {
    std::string value;
};

struct NoneLiteral {};

// Literal Statments

struct BreakStmt {};

struct ContinueStmt {};

// Variants

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
    Ptr<struct BlockExpr>
>;

using Stmt = Variant<
    Empty,
    BreakStmt,
    ContinueStmt,
    Ptr<struct ExprStmt>,
    Ptr<struct PrintStmt>,
    Ptr<struct IfStmt>,
    Ptr<struct LoopBlock>,
    Ptr<struct WhileLoop>,
    Ptr<struct ForLoop>,
    Ptr<struct ReturnStmt>,
    Ptr<struct FuncDeclaration>,
    Ptr<struct VarDeclaration>
>;

struct AssignmentExpr {
    Identifier target;
    Expr expr;
};

struct BinaryExpr {
    enum Operation {
        Add,
        Subtract,
        Multiply,
        Divide,
        Exponent,
        GreaterThan,
        LessThan,
        GreaterThanOrEq,
        LessThanOrEq,
        Equal,
        NotEqual
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

struct BlockExpr {
    std::vector<Stmt> body;
};

// Statements

struct ExprStmt {
    Expr expr;
};

struct PrintStmt {
    Expr expr;
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

// Declarations

struct FuncDeclaration {
    Identifier name;
    std::vector<Identifier> args;
    std::vector<Stmt> body;
};

struct VarDeclaration {
    Identifier name;
    Expr expr;
};

// Ast

struct Ast {
    std::vector<Stmt> body;
};