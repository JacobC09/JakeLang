#pragma once
#include <vector>
#include "variant.h"

// Literals

struct NumLiteral {
    int value;
};

// Variants

using Expr = Variant<
    NumLiteral,
    Ptr<struct AddExpr>
>;

using Stmt = Variant<
    Ptr<struct ExprStmt>
>;

// Expressions

struct AddExpr {
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