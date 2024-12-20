#include "compiler/compiler.h"
#include <cstring>

Chunk Compiler::compile(Ast& ast) {
    newChunk();
    hadError = false;
    chunkData->global = true;
    chunkData->localOffset = 0;
    body(ast.body);
    emitByte(OpExit, 0);
    return endChunk();
}

bool Compiler::failed() {
    return hadError;
}

Error Compiler::getError() {
    return error;
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
    chunkData->localOffset = 1;
}

Chunk Compiler::endChunk() {
    Chunk chunk = chunkData->chunk;
    chunkData = std::move(chunkData->enclosing);
    return chunk;
}

void Compiler::beginScope() {
    chunkData->scopeDepth++;
}

void Compiler::endScope() {
    u8 localCount = 0;
    for (auto& local : chunkData->locals) {
        if (local.depth < chunkData->scopeDepth) {
            break;
        }

        localCount++;
    }

    emitByte(OpPopLocals, localCount);

    chunkData->scopeDepth--;
    chunkData->locals.resize(chunkData->locals.size() - localCount);
}

void Compiler::beginLoop() {
    beginScope();
    std::unique_ptr<LoopData> enclosing = std::move(chunkData->loopData);
    chunkData->loopData = std::make_unique<LoopData>();
    chunkData->loopData->start = (signed)getChunk()->bytecode.size();
}

void Compiler::endLoop() {
    endScope();
    for (int where : chunkData->loopData->breaks) {
        patchJump(where);
    }
    chunkData->loopData = std::move(chunkData->loopData->enclosing);
}

void Compiler::errorAt(SourceView view, std::string msg, std::string note) {
    if (hadError) return;
    hadError = true;
    error = Error{view, "CompileError", msg, note, path};
}

void Compiler::internalError(std::string msg) {
    if (hadError) return;
    hadError = true;

    printf("Internal compilation error\n");
    printf("    %s\n", msg.c_str());
}

int Compiler::makeNumberConstant(double value, SourceView view) {
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
        errorAt(view, "Too many constants in pool");
    }

    return index;
}

int Compiler::makeNameConstant(std::string value, SourceView view) {
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
        errorAt(view, "Too many constants in pool");
    }

    return index;
}

void Compiler::addLocal(std::string name, SourceView view) {
    for (auto& local : chunkData->locals) {
        if (local.name == name && local.depth == chunkData->scopeDepth) {
            errorAt(view, formatStr("Already a local called '%s'", name));
            return;
        }
    }

    if (chunkData->locals.size() > UINT8_MAX) {
        errorAt(view, "Too many locals in scope");
        return;
    }

    chunkData->locals.push_back(Local{name, chunkData->scopeDepth});
}

int Compiler::addUpValue(std::unique_ptr<ChunkData>& chunk, u8 index, bool isLocal, SourceView view) {
    for (int i = 0; i < (signed)chunk->upValues.size(); i++) {
        auto& upValue = chunk->upValues[i];
        if (upValue.index == index && upValue.isLocal == isLocal) {
            return i;
        }
    }

    int count = chunk->upValues.size();
    if (count > UINT8_MAX) {
        errorAt(view, "Too many captured locals in scope");
        return -1;
    }

    chunk->upValues.push_back(UpValueData{index, isLocal});
    return count;
}

int Compiler::findLocal(std::unique_ptr<ChunkData>& chunk, std::string name) {
    for (int index = 0; index < (signed)chunk->locals.size(); index++) {
        if (chunk->locals[index].name == name) {
            return index + chunk->localOffset;
        }
    }

    return -1;
}

int Compiler::findUpValue(std::unique_ptr<ChunkData>& chunk, std::string name, SourceView view) {
    if (chunk->enclosing == nullptr) {
        return -1;
    }

    int local = findLocal(chunk->enclosing, name);
    if (local != -1) {
        return addUpValue(chunk, local, true, view);
    }

    int upValue = findUpValue(chunk->enclosing, name, view);
    if (upValue != -1) {
        return addUpValue(chunk, upValue, false, view);
    }

    return -1;
}

void Compiler::declare(std::string name, SourceView view) {
    if (chunkData->scopeDepth == 0) {
        emitByte(OpDefineGlobal);
        emitByte(makeNameConstant(name, view));
        return;
    }

    addLocal(name, view);
}

void Compiler::body(std::vector<Stmt>& stmts) {
    for (Stmt& stmt : stmts) {
        switch (stmt.which()) {
            case Stmt::which<BreakStmt>(): {
                breakStmt(stmt.get<BreakStmt>());
                break;
            }

            case Stmt::which<ContinueStmt>(): {
                continueStmt(stmt.get<ContinueStmt>());
                break;
            }

            case Stmt::which<ExitStmt>(): {
                exitStmt(stmt.get<ExitStmt>());
                break;
            }

            case Stmt::which<Ptr<ExprStmt>>(): {
                expression(stmt.get<Ptr<ExprStmt>>()->expr);
                emitByte(OpPop);
                break;
            }

            case Stmt::which<Ptr<ReturnStmt>>(): {
                returnStmt(stmt.get<Ptr<ReturnStmt>>());
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

            case Stmt::which<Ptr<TypeDeclaration>>(): {
                typeDeclaration(stmt.get<Ptr<TypeDeclaration>>());
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

            case Stmt::which<Ptr<BlockStmt>>(): {
                beginScope();
                body(stmt.get<Ptr<BlockStmt>>()->body);
                endScope();
                break;
            }

            default:
                internalError("Invalid statement");
                break;
        }
    }
}

void Compiler::breakStmt(BreakStmt& stmt) {
    if (chunkData->loopData == nullptr) {
        errorAt(stmt.view, "Cannot use break statement outside of loop");
        return;
    }

    chunkData->loopData->breaks.push_back(emitJumpForwards(OpJump));
}

void Compiler::continueStmt(ContinueStmt& stmt) {
    if (chunkData->loopData == nullptr) {
        errorAt(stmt.view, "Cannot use continue statement outside of loop");
        return;
    }

    emitJumpBackwards(OpJumpBack, chunkData->loopData->start);
}

void Compiler::exitStmt(ExitStmt& stmt) {
    if (stmt.code.value > UINT8_MAX) {
        errorAt(stmt.code.view, formatStr("Error code can't be greater than %d", UINT8_MAX));
        return;
    }
    emitByte(OpExit, (u8)stmt.code.value);
}

void Compiler::returnStmt(Ptr<ReturnStmt> stmt) {
    if (chunkData->global) {
        errorAt(stmt->view, "Return outside function");
        return;
    }

    expression(stmt->value);
    emitByte(OpSetLocal, 0, OpPop);
}

void Compiler::printStmt(Ptr<PrintStmt>& stmt) {
    for (int i = (signed)stmt->exprs.size() - 1; i >= 0; i--) {
        expression(stmt->exprs[i]);
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
    beginLoop();
    int start = (signed)getChunk()->bytecode.size();
    body(stmt->body);
    emitJumpBackwards(OpJumpBack, start);
    endLoop();
}

void Compiler::whileLoop(Ptr<WhileLoop>& stmt) {
    beginLoop();
    int start = (signed)getChunk()->bytecode.size();
    expression(stmt->condition);
    int endJump = emitJumpForwards(OpJumpPopIfFalse);
    body(stmt->body);
    emitJumpBackwards(OpJumpBack, start);
    patchJump(endJump);
    endLoop();
}

void Compiler::forLoop(Ptr<ForLoop>& stmt) {
    internalError("For loops are not supported yet");
}

void Compiler::typeDeclaration(Ptr<TypeDeclaration>& stmt) {
    emitByte(OpType, makeNameConstant(stmt->name.name, stmt->name.view));

    if (stmt->parents.size() > UINT8_MAX) {
        SourceView view = stmt->parents[UINT8_MAX].view | stmt->parents.back().view;
        errorAt(view, formatStr("Too many types to inherit from (max: %d, you have %d)", UINT8_MAX, stmt->parents.size()));
        return;
    }

    if (stmt->parents.size()) {
        for (auto& parent : stmt->parents) {
            identifier(parent, true);
        }
        
        emitByte(OpInherit, (u8)stmt->parents.size());
    }

    // Add code for methods
}

void Compiler::funcDeclaration(Ptr<FuncDeclaration>& stmt) {
    emitByte(OpFunction, (signed)getChunk()->constants.prototypes.size());
    newChunk();
    beginScope();

    chunkData->localOffset = 1;

    if (stmt->args.size() > UINT8_MAX) {
        SourceView view = stmt->args[UINT8_MAX].view | stmt->args.back().view;
        errorAt(view, formatStr("Too many arguments in function declaration (max: %d, you have %d)", UINT8_MAX, stmt->args.size()));
        return;
    }

    for (auto& arg : stmt->args) {
        addLocal(arg.name, arg.view);
    }

    body(stmt->body);
    endScope();
    emitByte(OpReturn);

    for (auto& upValue : chunkData->upValues) {
        chunkData->enclosing->chunk.bytecode.push_back(upValue.index);
        chunkData->enclosing->chunk.bytecode.push_back((u8)upValue.isLocal);
    }

    Prototype prot = {
        stmt->name.name,
        (u8)stmt->args.size(),
        (u8)chunkData->upValues.size(),
        endChunk(),
    };

    declare(stmt->name.name, stmt->name.view);
    getChunk()->constants.prototypes.push_back(prot);
}

void Compiler::varDeclaration(Ptr<VarDeclaration>& stmt) {
    if (stmt->expr.is<Empty>()) {
        emitByte(OpNone);
    } else {
        expression(stmt->expr);
    }
    declare(stmt->target.name, stmt->target.view);
}

void Compiler::expression(Expr expr) {
    switch (expr.which()) {
        case Expr::which<NumLiteral>(): {
            NumLiteral& num = expr.get<NumLiteral>();
            if (num.value > UINT8_MAX || num.value < 0) {
                int index = makeNumberConstant(num.value, num.view);
                emitByte(OpNumber, (u8)index);
            } else {
                emitByte(OpByteNumber, (u8)num.value);
            }
            break;
        }

        case Expr::which<BoolLiteral>(): {
            emitByte(expr.get<BoolLiteral>().value ? OpTrue : OpFalse);
            break;
        }

        case Expr::which<StrLiteral>(): {
            StrLiteral& str = expr.get<StrLiteral>();
            int index = makeNameConstant(str.value, str.view);
            emitByte(OpName, (u8)index);
            break;
        }

        case Expr::which<NoneLiteral>(): {
            emitByte(OpNone);
            break;
        }

        case Expr::which<Identifier>(): {
            identifier(expr.get<Identifier>(), true);
            break;
        }

        case Expr::which<Ptr<AssignmentExpr>>(): {
            assignment(expr.get<Ptr<AssignmentExpr>>());
            break;
        }

        case Expr::which<Ptr<BinaryExpr>>(): {
            auto& binaryExpr = expr.get<Ptr<BinaryExpr>>();
            marker(binaryExpr->opToken.view);

            if (binaryExpr->op == BinaryExpr::Operation::And) {
                expression(binaryExpr->left);
                int jump = emitJumpForwards(OpJumpIfFalse);
                emitByte(OpPop);
                expression(binaryExpr->right);
                patchJump(jump);
                break;
            } else if (binaryExpr->op == BinaryExpr::Operation::Or) {
                expression(binaryExpr->left);
                int jump = emitJumpForwards(OpJumpIfTrue);
                emitByte(OpPop);
                expression(binaryExpr->right);
                patchJump(jump);
                break;
            }

            expression(binaryExpr->left);
            expression(binaryExpr->right);

            switch (binaryExpr->op) {
                case BinaryExpr::Operation::Add:
                    emitByte(OpAdd);
                    break;
                case BinaryExpr::Operation::Subtract:
                    emitByte(OpSubtract);
                    break;
                case BinaryExpr::Operation::Modulous:
                    emitByte(OpModulous);
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
                default:
                    break;
            }

            break;
        }

        case Expr::which<Ptr<UnaryExpr>>(): {
            auto& unaryExpr = expr.get<Ptr<UnaryExpr>>();
            expression(unaryExpr->expr);
            marker(unaryExpr->opToken.view);

            switch (unaryExpr->op) {
                case UnaryExpr::Operation::Negative:
                    makeNumberConstant(-1, unaryExpr->opToken.view);
                    emitByte(OpMultiply);
                    break;
                case UnaryExpr::Operation::Negate:
                    emitByte(OpNot);
                    break;
            }

            break;
        }

        case Expr::which<Ptr<CallExpr>>(): {
            auto& call = expr.get<Ptr<CallExpr>>();
            emitByte(OpNone);
            for (auto& expr : call->args) {
                expression(expr);
            }
            expression(call->target);

            if (call->args.size() > UINT8_MAX) {
                SourceView view = getSourceView(call->args[UINT8_MAX]);
                for (int i = UINT8_MAX + 1; i < (signed)call->args.size(); i++) {
                    view = view | getSourceView(call->args[i]);
                }

                errorAt(view, formatStr("Too many arguments in function call (max: %d)", UINT8_MAX));
            }

            marker(getSourceView(call->target));
            emitByte(OpCall);
            marker(call->view);
            emitByte(call->args.size());
            break;
        }

        case Expr::which<Ptr<PropertyExpr>>(): {
            auto& prop = expr.get<Ptr<PropertyExpr>>();
            expression(prop->expr);
            marker(prop->prop.view);
            emitByte(OpGetProperty, makeNameConstant(prop->prop.name, prop->prop.view));
            break;
        }

        case Expr::which<Empty>():
        default:
            internalError("Invalid expression");
            break;
    }
}

void Compiler::assignment(Ptr<AssignmentExpr>& assignment) {
    expression(assignment->expr);

    switch (assignment->target.which()) {
        case Expr::which<Identifier>(): {
            identifier(assignment->target.get<Identifier>(), false);
            return;
        }

        case Expr::which<Ptr<PropertyExpr>>(): {
            auto& prop = assignment->target.get<Ptr<PropertyExpr>>();
            expression(prop->expr);
            emitByte(OpSetProperty, makeNameConstant(prop->prop.name, prop->prop.view));
            break;
        }

        default:
            break;
    }
}

void Compiler::identifier(Identifier& id, bool get) {
    int local = findLocal(chunkData, id.name);
    if (local != -1) {
        emitByte(get ? OpGetLocal : OpSetLocal, local);
        return;
    }

    int upValue = findUpValue(chunkData, id.name, id.view);
    if (upValue != -1) {
        emitByte(get ? OpGetUpValue : OpSetUpValue, upValue);
        return;
    }

    marker(id.view);
    emitByte(get ? OpGetGlobal : OpSetGlobal);
    emitByte(makeNameConstant(id.name, id.view));
}

void Compiler::emitByte(u8 value) {
    getChunk()->bytecode.push_back(value);
}

void Compiler::emitByte(u16 value) {
    emitByte((u8)((value >> 8) & 0xff));
    emitByte((u8)(value & 0xff));
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

void Compiler::marker(SourceView & view) {
    getChunk()->markers.push_back({getChunk()->bytecode.size(), view});
}

void Compiler::emitJumpBackwards(u8 jump, int where) {
    int distance = getChunk()->bytecode.size() - where + 2;
    if (distance > UINT16_MAX) {
        internalError("Condition jump too large");
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
        internalError("Condition jump too large");
        return;
    }

    getChunk()->bytecode[index] = (u8)((distance >> 8) & 0xff);
    getChunk()->bytecode[index + 1] = (u8)(distance & 0xff);
}
