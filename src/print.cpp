#include "print.h"

#include "compiler/compiler.h"
#include "debug.h"
#include "syntax/ast.h"

void printStmt(const Stmt& stmt, int indent);
int disassembleInstruction(const Chunk& chunk, int index);

void printToken(const Token& token) {
    static const char* names[] = {
        "LeftParen", "RightParen",
        "LeftBrace", "RightBrace",
        "Comma", "Dot", "Plus", "Minus",
        "Slash", "Asterisk", "Carret", "Semicolon",
        "Percent",

        "Bang", "BangEqual",
        "Equal", "EqualEqual",
        "Greater", "GreaterEqual",
        "Less", "LessEqual", "PlusEqual",
        "MinusEqual", "AsteriskEqual", "SlashEqual",
        "CarretEqual",

        "Identifier", "String", "Number", "True", "False", "None",

        "Print", "If", "Else", "Loop", "While", "For", "In", "Continue",
        "Break", "Func", "Var", "Exit", "And", "Or",

        "Error", "EndOfFile"};

    if (token.value.length() > 0) {
        printf("Token{type=%s, value=\'%s\'}\n", names[(int)token.type], std::string(token.value).c_str());
    } else {
        print((int)token.type);
        printf("Token{type=%s}\n", names[(int)token.type]);
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
            double val = expr.get<NumLiteral>().value;
            if (std::floor(val) != val) {
                printf("NumLiteral{%f}\n", val);
            } else {
                printf("NumLiteral{%d}\n", (int)val);
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
            printf("Identifier{%s}\n", expr.get<Identifier>().name.c_str());
            break;
        }

        case Expr::which<Ptr<AssignmentExpr>>(): {
            auto val = expr.get<Ptr<AssignmentExpr>>();

            print("AssigmentExpr{}");
            printIndent(indent + 1);
            print("Target:");
            printExpr(val->target, indent + 1);
            printExpr(val->expr, indent + 1);
            break;
        }

        case Expr::which<Ptr<BinaryExpr>>(): {
            auto val = expr.get<Ptr<BinaryExpr>>();

            static const char* names[] = {
                "Add", "Subtract", "Modulo", "Multiply",
                "Divide", "Exponent", "GreaterThan",
                "LessThan", "GreaterThanOrEq",
                "LessThanOrEq", "Equal", "NotEqual", "And", "Or"};

            printf("BinaryExpr{%s}\n", names[(int)val->op]);
            printExpr(val->left, indent + 1);
            printExpr(val->right, indent + 1);
            break;
        }

        case Expr::which<Ptr<UnaryExpr>>(): {
            auto val = expr.get<Ptr<UnaryExpr>>();

            static const char* names[] = {
                "Negative",
                "Negate",
            };

            printf("BinaryExpr{%s}\n", names[(int)val->op]);
            printExpr(val->expr, indent + 1);
            break;
        }

        case Expr::which<Ptr<CallExpr>>(): {
            auto& val = expr.get<Ptr<CallExpr>>();
            print("CallExpr{}");
            printIndent(indent + 1);
            print("Args:", val->args.size() ? "" : "(none)");
            for (auto& expr : val->args) {
                printExpr(expr, indent + 2);
            }
            printExpr(val->target, indent + 1);
            break;
        }

        case Expr::which<Ptr<PropertyExpr>>(): {
            auto& val = expr.get<Ptr<PropertyExpr>>();
            printf("PropertyExpr{%s}\n", val->prop.name.c_str());
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
            print(val->target.name);
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
            print(val->name.name);
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
            printf("ValDeclaration{%s}\n", val->target.name.c_str());
            printExpr(val->expr, indent + 1);
            break;
        }

        case Stmt::which<Ptr<BlockStmt>>(): {
            print("BlockExpr{}");
            for (auto& stmt : stmt.get<Ptr<BlockStmt>>()->body) {
                printStmt(stmt, indent + 1);
            }
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

int constantInstruction(const char* name, int index, const Chunk& chunk, bool isName = false) {
    int constant = chunk.bytecode[index + 1];

    if (isName) {
        printf("%-16s %s (%d)\n", name, chunk.constants.names[constant].c_str(), constant);
    } else {
        double val = chunk.constants.numbers[constant];
        if (val == (int)val) {
            printf("%-16s %4d (%d)\n", name, (int)val, constant);
        } else {
            printf("%-16s %4lf (%d)\n", name, val, constant);
        }
    }

    return index + 2;
}

int byteInstruction(const char* name, int index, const Chunk& chunk) {
    printf("%-16s %4d\n", name, chunk.bytecode[index + 1]);
    return index + 2;
}

int jumpInstruction(const char* name, int index, const Chunk& chunk, bool back = false) {
    int val = chunk.bytecode[index + 1] << 8 | chunk.bytecode[index + 2];
    printf("%-16s %4d to %d\n", name, val, index + (back ? -val : val) + 3);
    return index + 3;
}

int functionInstruction(const char* name, int index, const Chunk& chunk) {
    int prototypeIndex = chunk.bytecode[++index];
    const Prototype& prototype = chunk.constants.prototypes[prototypeIndex];
    printf("%-16s %4d, argc: %d\n", name, prototypeIndex, prototype.argc);
    printf(">=== %s ===<\n", prototype.name.c_str());
    for (int i = 0; i < prototype.upValues; i++) {
        u8 upValueIndex = chunk.bytecode[++index];
        u8 isLocal = chunk.bytecode[++index];
        printf("UpValue >> index: %d, isLocal: %s\n", upValueIndex, isLocal ? "true" : "false");
    }

    for (int i = 0; i < (signed)prototype.chunk.bytecode.size();) {
        i = disassembleInstruction(prototype.chunk, i);
    }

    printf(">====%s====<\n", std::string(prototype.name.size(), '=').c_str());

    return index + 1;
}

int disassembleInstruction(const Chunk& chunk, int index) {
    printf("%04d ", index);

    switch (chunk.bytecode[index]) {
        case OpExit:
            index = byteInstruction("Exit", index, chunk);
            break;
        case OpReturn:
            index = simpleInstruction("Return", index);
            break;
        case OpPop:
            index = simpleInstruction("Pop", index);
            break;
        case OpNumber:
            index = constantInstruction("Number", index, chunk, false);
            break;
        case OpName:
            index = constantInstruction("Name", index, chunk, true);
            break;
        case OpByteNumber:
            index = byteInstruction("ByteNumber", index, chunk);
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
        case OpModulous:
            index = simpleInstruction("Modulous", index);
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
        case OpGetProperty:
            index = constantInstruction("GetProperty", index, chunk, true);
            break;
        case OpSetProperty:
            index = constantInstruction("SetProperty", index, chunk, true);
            break;
        case OpGetUpValue:
            index = byteInstruction("GetUpValue", index, chunk);
            break;
        case OpSetUpValue:
            index = byteInstruction("SetUpValue", index, chunk);
            break;
        case OpPopLocals:
            index = byteInstruction("CloseUpValue", index, chunk);
            break;
        case OpJump:
            index = jumpInstruction("Jump", index, chunk);
            break;
        case OpJumpBack:
            index = jumpInstruction("JumpBack", index, chunk, true);
            break;
        case OpJumpIfTrue:
            index = jumpInstruction("JumpIfTrue", index, chunk);
            break;
        case OpJumpIfFalse:
            index = jumpInstruction("JumpIfFalse", index, chunk);
            break;
        case OpJumpPopIfFalse:
            index = jumpInstruction("JumpPopIfFalse", index, chunk);
            break;
        case OpFunction:
            index = functionInstruction("Function", index, chunk);
            break;
        case OpCall:
            index = byteInstruction("Call", index, chunk);
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

    for (int index = 0; index < (signed)chunk.bytecode.size();) {
        index = disassembleInstruction(chunk, index);
    }

    printf(">====%s====<\n", std::string(name.size(), '=').c_str());
}

std::string getTypename(int which) {
    switch (which) {
        case Value::which<Number>():
            return "Number";
        case Value::which<String>():
            return "String";
        case Value::which<Boolean>():
            return "Boolean";
        case Value::which<None>():
            return "None";
        case Value::which<Shared<UpValue>>():
            return "UpValue";
        case Value::which<Shared<Function>>():
            return "Function";
        case Value::which<Shared<Module>>():
            return "Module";
        default:
            return "Unknown";
    }
}

std::string getValueStr(const Value& value) {
    switch (value.which()) {
        case Value::which<Number>(): {
            return formatStr("%lf", value.get<Number>());
        }
        case Value::which<String>(): {
            return value.get<String>();
        }
        case Value::which<Boolean>(): {
            return value.get<bool>() ? "true" : "false";
        }
        case Value::which<None>(): {
            return "None";
        }
        case Value::which<Shared<UpValue>>(): {
            auto& val = value.get<Shared<UpValue>>();
            return formatStr("UpValue{%s}", getValueStr(*val->loc));
        }
        case Value::which<Shared<Function>>(): {
            auto val = value.get<Shared<Function>>();
            return formatStr("Function{%s, argc: %d}", val->prot.name, val->prot.argc);
        }
        case Value::which<Shared<BuiltInFunction>>(): {
            auto val = value.get<Shared<BuiltInFunction>>();
            return formatStr("BuiltInFunction{%s}", val->name);
        }
        case Value::which<Shared<Module>>(): {
            auto& val = value.get<Shared<Module>>();
            return formatStr("Module{%s}", val->name);
        }
        default:
            return "Unknown{}";
    }
}

void printValue(const Value& value) {
    print(getValueStr(value));
}
