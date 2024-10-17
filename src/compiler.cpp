#include "compiler.h"

#include <cstring>

Chunk Compiler::compile(Ast& ast) {
    hadError = false;
    newChunk();
    chunkData->global = true;
    body(ast.body);
    endChunk();
    return chunkData->chunk;
}

Chunk* Compiler::getChunk() {
    return &chunkData->chunk;
}

bool Compiler::failed() {
    return hadError;
}

void Compiler::error(std::string msg) {
    if (hadError) return;

    hadError = true;
    printf("Error during compilation\n");
    printf("    %s\n", msg.c_str());
}

void Compiler::emitByte(u8 value) {
    getChunk()->bytecode.push_back(value);
}

void Compiler::emitByte(u16 value) {
    emitByte((u8)((value >> 8) & 0xff));
    emitByte((u8)(value & 0xff));
}

int Compiler::emitJump(u8 jump) {
    emitByte(jump, 0, 0);
    return getChunk()->bytecode.size() - 2;
}

void Compiler::patchJump(int index) {
    int to = getChunk()->bytecode.size() - 1;
    if (to > UINT16_MAX) {
        error("Condition jump too large");
        return;
    }

    getChunk()->bytecode[index] = (u8)((to >> 8) & 0xff);
    getChunk()->bytecode[index + 1] = (u8)(to & 0xff);
}

template <typename First>
void Compiler::emitByte(First value) {
    emitByte((u8)value);
}

template <typename First, typename... Rest>
void Compiler::emitByte(First byte, Rest... rest) {
    emitByte(byte);
    emitByte(rest...);
}

void Compiler::newChunk() {
    std::unique_ptr<ChunkData> enclosing = std::move(chunkData);
    chunkData = std::make_unique<ChunkData>();
    chunkData->enclosing = std::move(enclosing);
    chunkData->scopeDepth = 0;
    chunkData->global = false;
}

void Compiler::endChunk() {
    emitByte(OpReturn);
}

void Compiler::body(std::vector<Stmt>& stmts) {
    for (Stmt& stmt : stmts) {
        switch (stmt.which()) {
            case Stmt::which<Ptr<ExprStmt>>(): {
                expression(stmt.get<Ptr<ExprStmt>>()->expr);
                emitByte(OpPop, (u8)1);
                break;
            }

            case Stmt::which<Ptr<ReturnStmt>>(): {
                auto val = stmt.get<Ptr<ReturnStmt>>();
                expression(val->value);
                emitByte(OpReturn);
                break;
            }

            case Stmt::which<Ptr<PrintStmt>>(): {
                printStmt(stmt.get<Ptr<PrintStmt>>());
                break;
            }

            case Stmt::which<Ptr<IfStmt>>(): {
                ifStmt(stmt.get<Ptr<IfStmt>>());
                break;
            }

            case Stmt::which<Ptr<LoopBlock>>(): {
                loopBlock(stmt.get<Ptr<LoopBlock>>());
                break;
            }

            case Stmt::which<Ptr<WhileLoop>>(): {
                whileLoop(stmt.get<Ptr<WhileLoop>>());
                break;
            }

            case Stmt::which<Ptr<ForLoop>>(): {
                forLoop(stmt.get<Ptr<ForLoop>>());
                break;
            }

            case Stmt::which<Ptr<FuncDeclaration>>(): {
                funcDeclaration(stmt.get<Ptr<FuncDeclaration>>());
                break;
            }

            case Stmt::which<Ptr<VarDeclaration>>(): {
                varDeclaration(stmt.get<Ptr<VarDeclaration>>());
                break;
            }

            default:
                error("Invalid statement");
                break;
        }
    }
}

void Compiler::expression(Expr expr) {
    switch (expr.which()) {
        case Expr::which<NumLiteral>(): {
            int index = makeNumberConstant(expr.get<NumLiteral>().value);
            emitByte(OpConstantNumber, (u8)index);
            break;
        }

        case Expr::which<BoolLiteral>(): {
            emitByte(expr.get<BoolLiteral>().value ? OpTrue : OpFalse);
            break;
        }

        case Expr::which<StrLiteral>(): {
            int index = makeNameConstant(expr.get<StrLiteral>().value);
            emitByte(OpConstantName, (u8)index);
            break;
        }

        case Expr::which<NoneLiteral>(): {
            emitByte(OpNone);
            break;
        }

        case Expr::which<Identifier>(): {
            identifier(expr.get<Identifier>());
            break;
        }

        case Expr::which<Ptr<AssignmentExpr>>(): {
            assignment(expr.get<Ptr<AssignmentExpr>>());
            break;
        }

        case Expr::which<Ptr<BinaryExpr>>(): {
            auto binaryExpr = expr.get<Ptr<BinaryExpr>>();

            expression(binaryExpr->left);
            expression(binaryExpr->right);

            switch (binaryExpr->op) {
                case BinaryExpr::Operation::Add:
                    emitByte(OpAdd);
                    break;
                case BinaryExpr::Operation::Subtract:
                    emitByte(OpSubtract);
                    break;
                case BinaryExpr::Operation::Multiply:
                    emitByte(OpMultiply);
                    break;
                case BinaryExpr::Operation::Divide:
                    emitByte(OpDivide);
                    break;
                case BinaryExpr::Operation::Exponent:
                    emitByte(OpExponent);
                    break;
                case BinaryExpr::Operation::GreaterThan:
                    emitByte(OpGreater);
                    break;
                case BinaryExpr::Operation::LessThan:
                    emitByte(OpLess);
                    break;
                case BinaryExpr::Operation::GreaterThanOrEq:
                    emitByte(OpGreaterThanOrEq);
                    break;
                case BinaryExpr::Operation::LessThanOrEq:
                    emitByte(OpLessThanOrEq);
                    break;
                case BinaryExpr::Operation::Equal:
                    emitByte(OpEqual);
                    break;
                case BinaryExpr::Operation::NotEqual:
                    emitByte(OpEqual);
                    emitByte(OpNot);
                    break;
            }

            break;
        }

        case Expr::which<Ptr<UnaryExpr>>(): {
            auto unaryExpr = expr.get<Ptr<UnaryExpr>>();

            expression(unaryExpr->expr);

            switch (unaryExpr->op) {
                case UnaryExpr::Operation::Negative:
                    makeNumberConstant(-1);
                    emitByte(OpMultiply);
                    break;
                case UnaryExpr::Operation::Negate:
                    emitByte(OpNot);
                    break;
            }
        }

        case Expr::which<Ptr<BlockExpr>>(): {
            auto block = expr.get<Ptr<BlockExpr>>();
            beginScope();
            body(block->body);
            endScope();
            break;
        }

        case Expr::which<Empty>():
        default:
            error("Invalid expression");
            break;
    }
}

void Compiler::assignment(Ptr<AssignmentExpr>& assignment) {
    expression(assignment->expr);

    int arg = findLocal(assignment->target.value);
    if (arg == -1) {
        emitByte(OpSetGlobal);
        emitByte(makeNameConstant(assignment->target.value));
    } else {
        emitByte(OpSetLocal, arg);
    }
}

void Compiler::identifier(Identifier& id) {
    int arg = findLocal(id.value);

    if (arg == -1) {
        emitByte(OpGetGlobal);
        emitByte(makeNameConstant(id.value));
    } else {
        emitByte(OpGetLocal, arg);
    }
}

void Compiler::printStmt(Ptr<PrintStmt>& stmt) {
    for (auto& expr : stmt->exprs) {
        expression(expr);
    }
    emitByte(OpPrint, stmt->exprs.size());
}

void Compiler::ifStmt(Ptr<IfStmt>& stmt) {
    expression(stmt->condition);
    int elseJump = emitJump(OpJumpIfFalse);
    emitJump(OpPop);
    body(stmt->body);
    int endJump = emitJump(OpJump);
    patchJump(elseJump);
    emitByte(OpPop);
    body(stmt->orelse);
    patchJump(endJump);
}

void Compiler::loopBlock(Ptr<LoopBlock>& stmt) {
}

void Compiler::whileLoop(Ptr<WhileLoop>& stmt) {
}

void Compiler::forLoop(Ptr<ForLoop>& stmt) {
}
void Compiler::funcDeclaration(Ptr<FuncDeclaration>& stmt) {
}

void Compiler::varDeclaration(Ptr<VarDeclaration>& stmt) {
    expression(stmt->expr);

    if (chunkData->scopeDepth == 0) {
        emitByte(OpDefineGlobal);
        emitByte(makeNameConstant(stmt->name.value));
        return;
    }

    addLocal(stmt->name.value);
}

int Compiler::makeNumberConstant(double value) {
    auto& pool = getChunk()->constants.numbers;
    auto it = std::find(pool.begin(), pool.end(), value);
    int index;
    if (it == pool.end()) {
        pool.push_back(value);
        index = pool.size() - 1;
    } else {
        index = std::distance(pool.begin(), it);
    }

    if (index > UINT8_MAX) {
        error("Too many constants in pool");
    }

    return index;
}

int Compiler::makeNameConstant(std::string value) {
    auto& pool = getChunk()->constants.names;
    auto it = std::find(pool.begin(), pool.end(), value);
    int index;
    if (it == pool.end()) {
        pool.push_back(value);
        index = (signed)pool.size() - 1;
    } else {
        index = std::distance(pool.begin(), it);
    }

    if (index > UINT8_MAX) {
        error("Too many constants in pool");
    }

    return index;
}

void Compiler::addLocal(std::string name) {
    for (auto& local : chunkData->locals) {
        if (local.name == name && local.depth == chunkData->scopeDepth) {
            error(formatStr("Already a local called '%s'", name.c_str()));
            break;
        }
    }

    chunkData->locals.push_back(Local{
        name, chunkData->scopeDepth});
}

int Compiler::findLocal(std::string name) {
    for (int index = 0; index < (signed)chunkData->locals.size(); index++) {
        if (chunkData->locals[index].name == name) {
            return index;
        }
    }

    return -1;
}

void Compiler::beginScope() {
    chunkData->scopeDepth++;
}

void Compiler::endScope() {
    chunkData->scopeDepth++;
    emitByte(OpPop, (u8)chunkData->locals.size());
    chunkData->locals.clear();
}
