#include "compiler.h"

Chunk Compiler::compile(Ast& ast) {
    hadError = false;
    newChunk();
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

template <typename ByteSize>
void Compiler::emitByte(ByteSize byte) {
    emitByte((u8) byte);
}

void Compiler::emitByte(u8 byte) {
    getChunk()->bytecode.push_back(byte);
}

void Compiler::emitByte(u16 longByte) {
    emitByte((u8) ((longByte >> 8) & 0xff));
    emitByte((u8) (longByte & 0xff));
}

template <typename First, typename... Bytes>
void Compiler::emitBytes(First byte, Bytes ... rest) {
    emitByte(byte);
    emitBytes(rest...);
}

template <typename First>
void Compiler::emitBytes(First byte) {
    emitByte(byte);
}

void Compiler::newChunk() {
    std::unique_ptr<ChunkData> enclosing = std::move(chunkData);
    chunkData = std::make_unique<ChunkData>();
    chunkData->enclosing = std::move(enclosing);
    chunkData->scopeDepth = 0;
}

void Compiler::endChunk() {
    emitByte(OpReturn);
}

void Compiler::body(std::vector<Stmt>& stmts) {
    for (Stmt& stmt : stmts) {
        switch (stmt.which()) {
            case Stmt::which<Ptr<ExprStmt>>(): {
                expression(stmt.get<Ptr<ExprStmt>>()->expr);
                emitBytes(OpPop, 1);
                break;
            }

            case Stmt::which<Ptr<VarDeclaration>>(): {
                
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
            if (index > UINT8_MAX) {
                emitBytes(OpConstantNumberDouble, (u16) index);
            } else {
                emitBytes(OpConstantNumber, (u8) index);
            }
            break;
        }

        case Expr::which<BoolLiteral>(): {
            emitByte(expr.get<BoolLiteral>().value ? OpTrue : OpFalse);
            break;
        }

        case Expr::which<StrLiteral>(): {
            int index = makeNameConstant(expr.get<StrLiteral>().value);
            if (index > UINT8_MAX) {
                emitBytes(OpConstantNameDouble, (u16) index);
            } else {
                emitBytes(OpConstantName, (u8) index);
            }
            break;
        }

        case Expr::which<NoneLiteral>(): {
            emitByte(OpNone);
            break;
        }

        case Expr::which<Identifier>(): {
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

        case Expr::which<Empty>():
        default:
            error("Invalid expression");
            break;
    }
}

void Compiler::assignment(Ptr<AssignmentExpr>& expr) {
    int arg = findLocal(expr->target.value);

    u8 get, set;
    if (arg == -1) {
        get = OpGetGlobal;
        set = OpSetGlobal;
        // arg = emitNameConstant()
    } else {
        get = OpGetLocal;
        set = OpSetLocal;
    }
}

void Compiler::varDeclaration(Ptr<VarDeclaration>& declaration) {
    if (chunkData->scopeDepth == 0) {
        expression(declaration->expr);
    }

    // This also emits the constant operation
    makeNameConstant(declaration->name.value);
    emitByte(OpDefineGlobal);
}

int Compiler::makeNumberConstant(double value) {
    auto& pool = getChunk()->constants.numbers;
    auto it = std::find(pool.begin(), pool.end(), value);
    
    if (it == pool.end()) {
        pool.push_back(value);
        return pool.size() - 1;
    } else {
        return std::distance(pool.begin(), it);
    }
}

int Compiler::makeNameConstant(std::string value) {
    auto& pool = getChunk()->constants.names;
    auto it = std::find(pool.begin(), pool.end(), value);
    
    if (it == pool.end()) {
        pool.push_back(value);
        return (signed) pool.size() - 1;
    } else {
        return std::distance(pool.begin(), it);
    }
}

void Compiler::addLocal(std::string name) {
    for (auto& local : chunkData->locals) {
        if (local.name == name && local.depth == chunkData->scopeDepth) {
            error(formatStr("Already a local called '%s'", name.c_str()));
            break;
        }
    }

    chunkData->locals.push_back(Local {
        name, chunkData->scopeDepth
    });
}

int Compiler::findLocal(std::string name) {
    for (int index = 0; index < (signed) chunkData->locals.size(); index++) {
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
    emitBytes(OpPop, (u8) chunkData->locals.size());
    chunkData->locals.clear();
}
