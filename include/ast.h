#pragma once
#include <vector>
#include "variant.h"

// Empty

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

// Variants

using Expr = Variant<
    Empty,
    NumLiteral,
    BoolLiteral,
    StrLiteral,
    NoneLiteral,
    Identifier,
    Ptr<struct BinaryExpr>
>;

using Stmt = Variant<
    Ptr<struct ExprStmt>
>;

struct BinaryExpr {
    enum Operation {
        Addition,
        Subtraction,
        Multiplication,
        Division,
        Exponentiation
    };

    Operation operator;
    Expr left;
    Expr right;
};

// Statements

struct ExprStmt {
    Expr expr;
};

// Ast

struct Ast {
    std::vector<Stmt> body;
};