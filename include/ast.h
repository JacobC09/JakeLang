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
    Ptr<struct BinaryExpr>,
    Ptr<struct UnaryExpr>
>;

using Stmt = Variant<
    Ptr<struct ExprStmt>
>;

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

// Statements

struct ExprStmt {
    Expr expr;
};

// Ast

struct Ast {
    std::vector<Stmt> body;
};