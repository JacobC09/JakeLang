#include "ast.h"
#include "print.h"
#include "debug.h"

void printToken(const Token& token) {
    static const char* names[] = {
        "LeftParen", "RightParen",
        "LeftBrace", "RightBrace",
        "Comma", "Dot", "Plus", "Minus",
        "Slash", "Asterisk", "Semicolon",

        "Bang", "BangEqual",
        "Equal", "EqualEqual",
        "Greater", "GreaterEqual",
        "Less", "LessEqual", "PlusEqual", "MinusEqual", "SlashEqual", "AsteriskEqual",

        "Identifier", "String", "Number", "True", "False", "None",

        "Error", "EndOfFile"
    };

    if (token.value.length() > 0) {
        printf("Token{type=%s, value=\'%s\'}\n", names[(int) token.type], std::string(token.value).c_str());
    } else {
        print((int) token.type);
        printf("Token{type=%s}\n", names[(int) token.type]);
    }
}

void printExpr(Expr& expr, int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }

    switch (expr.which()) {
        case Expr::which<NumLiteral>(): {
            auto val = expr.get<NumLiteral>();
            printf("NumLiteral{%f}\n", val.value);
            break;
        }

        case Expr::which<Ptr<BinaryExpr>>(): {
            auto val = expr.get<Ptr<BinaryExpr>>();
            print("BinaryExpr{}");

            static const char* names[] = { 
                "Add", "Subtract", "Multiply", 
                "Divide", "Exponent", "GreaterThan", 
                "LessThan", "GreaterThanOrEq", 
                "LessThanOrEq", "Equal", "NotEqual"
            };

            printf("Operator: %s\n", names[(int) val->op]);
            printExpr(val->left, indent + 1);
            printExpr(val->right, indent + 1);
            break;
        }

        case Expr::which<Ptr<UnaryExpr>>(): {
            auto val = expr.get<Ptr<UnaryExpr>>();
            print("BinaryExpr{}");

            static const char* names[] = { 
                "Negative", "Negate",
            };

            printf("Operator: %s\n", names[(int) val->op]);
            printExpr(val->expr, indent + 1);
            break;
        }
        
        default:
            break;
    }
}

void printStmt(Stmt& stmt, int indent) {
    for (int i = 0; i < indent; i++) {
        print("  ");
    }

    switch (stmt.which()) {
        case Stmt::which<Ptr<ExprStmt>>(): {
            auto val = stmt.get<Ptr<ExprStmt>>();
            print("ExprStmt{}");
            printExpr(val->expr, indent + 1);
            break;
        }

        default:
            break;
    }
}

void printAst(Ast& ast) {
    for (auto stmt : ast.body) {
        printStmt(stmt, 0);
    }
}
