#include "ast.h"
#include "print.h"
#include "debug.h"

void printStmt(const Stmt& stmt, int indent);

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

        "Print", "If", "Else", "Loop", "While", "For", "In", "Continue", "Break", "Func", "Var",

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

            printf("AssigmentExpr{%s}\n", val->target.value.c_str());
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

        case Expr::which<Ptr<BlockExpr>>(): {
            auto val = expr.get<Ptr<BlockExpr>>();
            print("BlockExpr{}");
            for (auto& stmt : val->body) {
                printStmt(stmt, indent + 1);
            }
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
            for (auto& expr : val->exprs) {
                printExpr(expr, indent + 1);
            }
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
            printIndent(indent + 2);
            print(val->target.value);
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
            printExpr(val->value, indent + 1);

            break;
        }

        case Stmt::which<Ptr<FuncDeclaration>>(): {
            auto val = stmt.get<Ptr<FuncDeclaration>>();
            print("FuncDeclaration{}");
            printIndent(indent + 1);
            print("Name:");
            printIndent(indent + 2);
            print(val->name.value);
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

        case Stmt::which<Ptr<VarDeclaration>>(): {
            auto val = stmt.get<Ptr<VarDeclaration>>();
            printf("ValDeclaration{%s}\n", val->name.value.c_str());
            printExpr(val->expr, indent + 1);
            break;
        }


        case Stmt::which<Empty>(): {
            print("Empty{}");
            break;
        }

        default:
            print("UnknownStmt{}");
            break;
    }
}

void printAst(const Ast& ast) {
    print(">=== Ast ===<");
    print("Ast{}");
    for (auto& stmt : ast.body) {
        printStmt(stmt, 1);
    }
    print(">===========<");
}

int simpleInstruction(const char* name, int index) {
    printf("%s\n", name);
    return index + 1;
}

int constantInstruction(const char* name, int index, const Chunk& chunk, bool isName=false) {
    int constant = chunk.bytecode[index + 1];

    if (isName) {
        printf("%-16s %s (%d)\n", name, chunk.constants.names[constant].c_str(), constant);
    } else {
        double val = chunk.constants.numbers[constant];
        if (val == (int) val) {
            printf("%-16s %d (%d)\n", name, (int) val, constant);
        } else {
            printf("%-16s %lf (%d)\n", name, val, constant);
        }
    }

    return index + 2;

}

int byteInstruction(const char* name, int index, const Chunk& chunk) {
    printf("%-16s %4d\n", name, chunk.bytecode[index + 1]);
    return index + 2;
}

int shortInstruction(const char* name, int index, const Chunk& chunk) {
    int val = chunk.bytecode[index + 1] << 8 | chunk.bytecode[index + 2];
    printf("%-16s %4d\n", name, val);
    return index + 3;
}

int disassembleInstruction(const Chunk& chunk, int index) {
    printf("%04d ", index);
    
    switch (chunk.bytecode[index]) {
        case OpPop:
            index = byteInstruction("Pop", index, chunk);
            break;
        case OpReturn:
            index = simpleInstruction("Return", index);
            break;
        case OpConstantNumber:
            index = constantInstruction("Number Constant", index, chunk, false);
            break;
        case OpConstantName:
            index = constantInstruction("Name Constant", index, chunk, true);
            break;
        case OpTrue:
            index = simpleInstruction("True", index);
            break;
        case OpFalse:
            index = simpleInstruction("False", index);
            break;
        case OpNone:
            index = simpleInstruction("None", index);
            break;
        case OpAdd:
            index = simpleInstruction("Add", index);
            break;
        case OpSubtract:
            index = simpleInstruction("Subtract", index);
            break;
        case OpMultiply:
            index = simpleInstruction("Multiply", index);
            break;
        case OpDivide:
            index = simpleInstruction("Divide", index);
            break;
        case OpExponent:
            index = simpleInstruction("Exponent", index);
            break;
        case OpEqual:
            index = simpleInstruction("Equal", index);
            break;
        case OpGreater:
            index = simpleInstruction("Greater", index);
            break;
        case OpLess:
            index = simpleInstruction("Less", index);
            break;
        case OpGreaterThanOrEq:
            index = simpleInstruction("GreaterThanOrEq", index);
            break;
        case OpLessThanOrEq:
            index = simpleInstruction("LessThanOrEq", index);
            break;
        case OpNot:
            index = simpleInstruction("Not", index);
            break;
        case OpNegate:
            index = simpleInstruction("Negate", index);
            break;
        case OpPrint:
            index = byteInstruction("Print", index, chunk);
            break;
        case OpDefineGlobal:
            index = constantInstruction("DefineGlobal", index, chunk, true);
            break;
        case OpGetGlobal:
            index = constantInstruction("GetGlobal", index, chunk, true);
            break;
        case OpSetGlobal:
            index = constantInstruction("SetGlobal", index, chunk, true);
            break;
        case OpGetLocal:
            index = byteInstruction("GetLocal", index, chunk);
            break;
        case OpSetLocal:
            index = byteInstruction("SetLocal", index, chunk);
            break;
        case OpGetUpValue:
            index = simpleInstruction("GetUpValue", index);
            break;
        case OpSetUpValue:
            index = simpleInstruction("SetUpValue", index);
            break;
        case OpCloseUpValue:
            index = simpleInstruction("CloseUpValue", index);
            break;
        case OpJump:
            index = shortInstruction("Jump", index, chunk);
            break;
        case OpJumpBack:
            index = shortInstruction("JumpBack", index, chunk);
            break;
        case OpJumpIfTrue:
            index = shortInstruction("JumpIfTrue", index, chunk);
            break;
        case OpJumpIfFalse:
            index = shortInstruction("JumpIfFalse", index, chunk);
            break;
        case OpCall:
            index = simpleInstruction("Call", index);
            break;
        case OpClosure:
            index = simpleInstruction("Closure", index);
            break;
        case OpClass:
            index = simpleInstruction("Class", index);
            break;
        case OpGetProperty:
            index = simpleInstruction("GetProperty", index);
            break;
        case OpSetProperty:
            index = simpleInstruction("SetProperty", index);
            break;
        case OpMethod:
            index = simpleInstruction("Method", index);
            break;
        case OpInvoke:
            index = simpleInstruction("Invoke", index);
            break;
        case OpInherit:
            index = simpleInstruction("Inherit", index);
            break;
        case OpGetSuper:
            index = simpleInstruction("OpGetSuper", index);
            break;

        default:
            print("Unknown Instruction");
            index++;
            break;
    }

    return index;
}

void printChunk(const Chunk& chunk, std::string name) {
    if (!name.size()) name = "Chunk";

    printf(">=== %s ===<\n", name.c_str());
    
    for (int index = 0; index < (signed) chunk.bytecode.size();) {
        index = disassembleInstruction(chunk, index);
    }

    printf(">====%s====<\n", std::string(name.size(), '=').c_str());

}
