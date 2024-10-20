#include "compiler.h"

#include <cstring>

Chunk Compiler::compile(Ast& ast) {
    hadError = false;
    newChunk();
    chunkData->global = true;
    body(ast.body);

    if (!ast.body.size() || ast.body.back().which() != Stmt::which<Ptr<ReturnStmt>>()) {
        emitByte(OpNone, OpReturn);
    }

    return endChunk();
}

bool Compiler::failed() {
    return hadError;
}

Chunk* Compiler::getChunk() {
    return &chunkData->chunk;
}

void Compiler::newChunk() {
    std::unique_ptr<ChunkData> enclosing = std::move(chunkData);
    chunkData = std::make_unique<ChunkData>();
    chunkData->enclosing = std::move(enclosing);
    chunkData->scopeDepth = 0;
    chunkData->global = false;
}

Chunk Compiler::endChunk() {
    Chunk chunk = chunkData->chunk;
    chunkData = std::move(chunkData->enclosing);
    return chunk;
}

void Compiler::newLoop() {
    std::unique_ptr<LoopData> enclosing = std::move(loopData);
    loopData = std::make_unique<LoopData>();
    loopData->start = (signed)getChunk()->bytecode.size();
}

void Compiler::endLoop() {
    for (int where : loopData->breaks) {
        patchJump(where);
    }
    loopData = std::move(loopData->enclosing);
}

void Compiler::error(std::string msg) {
    if (hadError) return;

    hadError = true;
    printf("Error during compilation\n");
    printf("    %s\n", msg.c_str());
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
            return;
        }
    }

    if (chunkData->locals.size() > UINT8_MAX) {
        error("Too many locals in scope");
        return;
    }

    chunkData->locals.push_back(Local{name, chunkData->scopeDepth});
}

int Compiler::addUpValue(std::unique_ptr<ChunkData>& chunk, u8 index, bool inEnclosingChunk) {
    for (int i = 0; i < (signed)chunk->upvalues.size(); i++) {
        auto& upvalue = chunk->upvalues[i];
        if (upvalue.index == index && upvalue.inEnclosingChunk == inEnclosingChunk) {
            return i;
        }
    }
    
    int count = chunk->upvalues.size();
    if (count > UINT8_MAX) {
        error("Too many captured locals in scope");
        return -1;
    }

    chunk->upvalues.push_back(UpValue {index, inEnclosingChunk});
    return count;
}

int Compiler::findLocal(std::unique_ptr<ChunkData>& chunk, std::string name) {
    for (int index = 0; index < (signed)chunk->locals.size(); index++) {
        if (chunk->locals[index].name == name) {
            return index;
        }
    }

    return -1;
}

int Compiler::findUpValue(std::unique_ptr<ChunkData>& chunk, std::string name) {
    if (chunk->enclosing == nullptr) {
        return -1;
    }

    int local = findLocal(chunkData->enclosing, name);
    if (local != -1) {
        return addUpValue(chunk, local, true);
    }

    int upvalue = findUpValue(chunk->enclosing, name);
    if (upvalue != -1) {
        return addUpValue(chunk, upvalue, false);
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

            case Stmt::which<BreakStmt>(): {
                breakStmt();
                break;
            }

            case Stmt::which<ContinueStmt>(): {
                continueStmt();
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

void Compiler::assignment(Ptr<AssignmentExpr>& assignment) {
    expression(assignment->expr);

    int local = findLocal(chunkData, assignment->target.value);
    if (local != -1) {
        emitByte(OpSetLocal, local);
        return;
    }

    int upvalue = findUpValue(chunkData, assignment->target.value);
    if (upvalue != -1) {
        emitByte(OpSetUpValue, upvalue);
        return;
    }

    emitByte(OpSetGlobal);
    emitByte(makeNameConstant(assignment->target.value));
}

void Compiler::printStmt(Ptr<PrintStmt>& stmt) {
    for (auto& expr : stmt->exprs) {
        expression(expr);
    }
    emitByte(OpPrint, stmt->exprs.size());
}

void Compiler::ifStmt(Ptr<IfStmt>& stmt) {
    expression(stmt->condition);
    int elseJump = emitJumpForwards(OpJumpPopIfFalse);
    body(stmt->body);

    if (stmt->orelse.size()) {
        int endJump = emitJumpForwards(OpJump);
        patchJump(elseJump);
        body(stmt->orelse);
        patchJump(endJump);
    } else {
        patchJump(elseJump);
    }
}

void Compiler::loopBlock(Ptr<LoopBlock>& stmt) {
    newLoop();
    int start = (signed)getChunk()->bytecode.size();
    body(stmt->body);
    emitJumpBackwards(OpJumpBack, start);
    endLoop();
}

void Compiler::whileLoop(Ptr<WhileLoop>& stmt) {
    newLoop();
    int start = (signed)getChunk()->bytecode.size();
    expression(stmt->condition);
    int endJump = emitJumpForwards(OpJumpPopIfFalse);
    body(stmt->body);
    emitJumpBackwards(OpJumpBack, start);
    patchJump(endJump);
    endLoop();
}

void Compiler::forLoop(Ptr<ForLoop>& stmt) {
    error("For loops are not supported yet");
}

void Compiler::breakStmt() {
    if (loopData == nullptr) {
        error("Cannot use break statement outside of loop");
        return;
    }

    loopData->breaks.push_back(emitJumpForwards(OpJump));
}

void Compiler::continueStmt() {
    if (loopData == nullptr) {
        error("Cannot use continue statement outside of loop");
        return;
    }

    emitJumpBackwards(OpJumpBack, loopData->start);
}

void Compiler::funcDeclaration(Ptr<FuncDeclaration>& stmt) {
    emitByte(OpFunction, (signed)getChunk()->constants.prototypes.size());
    newChunk();
    beginScope();

    for (auto& arg : stmt->args) {
        addLocal(arg.value);
    }

    body(stmt->body);
    endScope();

    Prototype prot = {
        stmt->name.value,
        (signed)stmt->args.size(),
        endChunk(),
    };

    getChunk()->constants.prototypes.push_back(prot);
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

void Compiler::expression(Expr expr) {
    switch (expr.which()) {
        case Expr::which<NumLiteral>(): {
            double value = expr.get<NumLiteral>().value;
            if (value > UINT8_MAX || value < 0) {
                int index = makeNumberConstant(value);
                emitByte(OpConstantNumber, (u8)index);
            } else {
                emitByte(OpByteNumber, (u8)value);
            }
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

            break;
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

void Compiler::identifier(Identifier& id) {
    int local = findLocal(chunkData, id.value);
    if (local != -1) {
        emitByte(OpGetLocal, local);
        return;
    }

    int upvalue = findUpValue(chunkData, id.value);
    if (upvalue != -1) {
        emitByte(OpGetUpValue, upvalue);
        return;
    }

    emitByte(OpGetGlobal);
    emitByte(makeNameConstant(id.value));
}

void Compiler::emitByte(u8 value) {
    getChunk()->bytecode.push_back(value);
}

void Compiler::emitByte(u16 value) {
    emitByte((u8)((value >> 8) & 0xff));
    emitByte((u8)(value & 0xff));
}

void Compiler::emitJumpBackwards(u8 jump, int where) {
    int distance = getChunk()->bytecode.size() - where + 1;
    if (distance > UINT16_MAX) {
        error("Condition jump too large");
        return;
    }

    emitByte(jump, (u16)distance);
}

int Compiler::emitJumpForwards(u8 jump) {
    emitByte(jump, 0, 0);
    return getChunk()->bytecode.size() - 2;
}

void Compiler::patchJump(int index) {
    int distance = getChunk()->bytecode.size() - index - 2;
    if (distance > UINT16_MAX) {
        error("Condition jump too large");
        return;
    }

    getChunk()->bytecode[index] = (u8)((distance >> 8) & 0xff);
    getChunk()->bytecode[index + 1] = (u8)(distance & 0xff);
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