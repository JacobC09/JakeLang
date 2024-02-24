#include "ast.h"
#include "print.h"
#include "debug.h"

void printToken(const Token& token) {
    static const char* names[] = {
        "LeftParen", "RightParen",
        "LeftBrace", "RightBrace",
        "Comma", "Dot", "Plus", "Minus",
        "Slash", "Asterisk", "Carret", "Semicolon",

        "Bang", "BangEqual",
        "Equal", "EqualEqual",
        "Greater", "GreaterEqual",
        "Less", "LessEqual", "PlusEqual", 
        "MinusEqual", "AsteriskEqual", "SlashEqual",
        "CarretEqual",

        "Identifier", "String", "Number", "True", "False", "None",

        "Print", "If", "Else", "Loop", "While", "For", "In", "Continue", "Break", "Func",

        "Error", "EndOfFile"
    };

    if (token.value.length() > 0) {
        printf("Token{type=%s, value=\'%s\'}\n", names[(int) token.type], std::string(token.value).c_str());
    } else {
        print((int) token.type);
        printf("Token{type=%s}\n", names[(int) token.type]);
    }
}

void printIndent(int indent) {
    for (int i = 0; i < indent; i++) {
        std::cout << "  ";
    }
}

void printExpr(const Expr& expr, int indent) {
    printIndent(indent);

    switch (expr.which()) {
        case Expr::which<NumLiteral>(): {
            auto val = expr.get<NumLiteral>();
            if (std::floor(val.value) != val.value) {
                printf("NumLiteral{%f}\n", val.value);
            } else {
                printf("NumLiteral{%d}\n", (int) val.value);
            }

            break;
        }

        case Expr::which<BoolLiteral>(): {
            printf("BoolLiteral{%s}\n", expr.get<BoolLiteral>().value ? "true" : "false");
            break;
        }

        case Expr::which<StrLiteral>(): {
            printf("StrLiteral{%s}\n", expr.get<StrLiteral>().value.c_str());
            break;
        }

        case Expr::which<NoneLiteral>(): {
            print("NoneLiteral{}");
            break;
        }

        case Expr::which<Identifier>(): {
            printf("Identifier{%s}\n", expr.get<Identifier>().value.c_str());
            break;
        }

        case Expr::which<Ptr<AssignmentExpr>>(): {
            auto val = expr.get<Ptr<AssignmentExpr>>();

            print("AssigmentExpr{}");
            printExpr(val->target, indent + 1);
            printExpr(val->expr, indent + 1);
            break;
        }

        case Expr::which<Ptr<BinaryExpr>>(): {
            auto val = expr.get<Ptr<BinaryExpr>>();
            
            static const char* names[] = { 
                "Add", "Subtract", "Multiply", 
                "Divide", "Exponent", "GreaterThan", 
                "LessThan", "GreaterThanOrEq", 
                "LessThanOrEq", "Equal", "NotEqual"
            };

            printf("BinaryExpr{%s}\n", names[(int) val->op]);
            printExpr(val->left, indent + 1);
            printExpr(val->right, indent + 1);
            break;
        }

        case Expr::which<Ptr<UnaryExpr>>(): {
            auto val = expr.get<Ptr<UnaryExpr>>();

            static const char* names[] = { 
                "Negative", "Negate",
            };

            printf("BinaryExpr{%s}\n", names[(int) val->op]);
            printExpr(val->expr, indent + 1);
            break;
        }

        case Expr::which<Empty>(): {
            print("Empty{}");
            break;
        }
        
        default:
            break;
    }
}

void printStmt(const Stmt& stmt, int indent) {
    printIndent(indent);

    switch (stmt.which()) {
        case Stmt::which<BreakStmt>(): {
            print("BreakStmt{}");
            break;
        }

        case Stmt::which<ContinueStmt>(): {
            print("ContinueStmt{}");
            break;
        }

        case Stmt::which<Ptr<ExprStmt>>(): {
            auto val = stmt.get<Ptr<ExprStmt>>();
            print("ExprStmt{}");
            printExpr(val->expr, indent + 1);
            break;
        }

        case Stmt::which<Ptr<PrintStmt>>(): {
            auto val = stmt.get<Ptr<PrintStmt>>();
            print("PrintStmt{}");
            printExpr(val->expr, indent + 1);
            break;
        }
        
        case Stmt::which<Ptr<IfStmt>>(): {
            auto val = stmt.get<Ptr<IfStmt>>();
            print("IfStmt{}");
            printIndent(indent + 1);
            print("Condition:");
            printExpr(val->condition, indent + 2);
            printIndent(indent + 1);
            print("Body:");

            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 2);
            }

            if (val->orelse.size()) {
                printIndent(indent + 1);
                print("OrElse:");
                for (auto& stmt : val->orelse) {
                    printStmt(stmt, indent + 2);
                }
            }

            break;
        }

        case Stmt::which<Ptr<LoopBlock>>(): {
            auto val = stmt.get<Ptr<LoopBlock>>();
            print("LoopBlock{}");

            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 1);
            }

            break;
        }

        case Stmt::which<Ptr<WhileLoop>>(): {
            auto val = stmt.get<Ptr<WhileLoop>>();
            print("WhileLoop{}");
            printIndent(indent + 1);
            print("Condition:");
            printExpr(val->condition, indent + 2);
            printIndent(indent + 1);
            print("Body:");

            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 2);
            }

            break;
        }

        case Stmt::which<Ptr<ForLoop>>(): {
            auto val = stmt.get<Ptr<ForLoop>>();
            print("ForLoop{}");
            printIndent(indent + 1);
            print("Target:");
            printExpr(Expr {val->target}, indent + 2);
            printIndent(indent + 1);
            print("Iterator:");
            printExpr(val->iterator, indent + 2);
            printIndent(indent + 1);
            print("Body:");

            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 2);
            }

            break;
        }

        case Stmt::which<Ptr<ReturnStmt>>(): {
            auto val = stmt.get<Ptr<ReturnStmt>>();
            print("ReturnStmt{}");
            for (auto& expr : val->values) {
                printExpr(expr, indent + 1);
            }

            break;
        }

        case Stmt::which<Ptr<FuncDeclaration>>(): {
            auto val = stmt.get<Ptr<FuncDeclaration>>();
            print("FuncDeclaration{}");
            printIndent(indent + 1);
            print("Name:");
            printExpr(Expr {val->name}, indent + 2);
            printIndent(indent + 1);
            print("Arguments:");
            for (auto& expr : val->args) {
                printExpr(expr, indent + 2);
            }
            printIndent(indent + 1);
            print("Body:");

            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 2);
            }

            break;
        }


        case Stmt::which<Empty>(): {
            print("Empty{}");
            break;
        }

        default:
            break;
    }
}

void printAst(const Ast& ast) {
    print("Ast{}");
    for (auto& stmt : ast.body) {
        printStmt(stmt, 1);
    }
}
