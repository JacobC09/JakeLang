#include "compiler.h"

Chunk Compiler::compile(Ast& ast) {
    hadError = false;
    newChunk();
    body(ast.body);
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

void Compiler::emitByte(u8 byte) {
    getChunk()->bytecode.push_back(byte);
}

void Compiler::newChunk() {
    std::unique_ptr<ChunkData> enclosing = std::move(chunkData);
    chunkData = std::make_unique<ChunkData>();
    chunkData->enclosing = std::move(enclosing);
    chunkData->scopeDepth = 0;
}

void Compiler::body(std::vector<Stmt>& stmts) {
    for (Stmt& stmt : stmts) {
        switch (stmt.which()) {
            case Stmt::which<Ptr<ExprStmt>>(): {
                expression(stmt.get<Ptr<ExprStmt>>()->expr);
                emitByte(OpPop);
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
            emitNumberConstant(expr.get<NumLiteral>().value);
            break;
        }

        case Expr::which<BoolLiteral>(): {
            emitByte(expr.get<BoolLiteral>().value ? OpTrue : OpFalse);
            break;
        }

        case Expr::which<StrLiteral>(): {
            emitNameConstant(expr.get<StrLiteral>().value);
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
                    emitNumberConstant(-1);
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

}

void Compiler::varDeclaration(Ptr<VarDeclaration>& declaration) {
}

void Compiler::emitNumberConstant(double value) {
    auto& pool = getChunk()->constants.numbers;
    auto it = std::find(pool.begin(), pool.end(), value);
    
    int index;
    if (it == pool.end()) {
        index = (signed) pool.size();
        pool.push_back(value);
    } else {
        index = std::distance(pool.begin(), it);
    }

    if (index > UINT8_MAX) {
        emitByte(OpConstantNumberDouble);
        emitByte((u8) ((index >> 8) & 0xff));
        emitByte((u8) (index & 0xff));
    } else {
        emitByte(OpConstantNumber);
        emitByte((u8) index);
    }
}

void Compiler::emitNameConstant(std::string value) {
    auto& pool = getChunk()->constants.names;
    auto it = std::find(pool.begin(), pool.end(), value);
    
    int index;
    if (it == pool.end()) {
        index = (signed) pool.size();
        pool.push_back(value);
    } else {
        index = std::distance(pool.begin(), it);
    }

    if (index > UINT8_MAX) {
        emitByte(OpConstantNameDouble);
        emitByte((u8) ((index >> 8) & 0xff));
        emitByte((u8) (index & 0xff));
    } else {
        emitByte(OpConstantName);
        emitByte((u8) index);
    }
}

void Compiler::addLocal(std::string name) {
    for (auto& local : chunkData->locals) {
        if (local.name == name) {
            error(formatStr("Already a local called '%s'", name.c_str()));
            break;
        }
    }

    chunkData->locals.push_back(Local {
        name, chunkData->scopeDepth
    });
}

void Compiler::beginScope() {
    chunkData->scopeDepth++;
}

void Compiler::endScope() {
    chunkData->scopeDepth++;
    emitBytes(OpPop, (u8) chunkData->locals.size());
    chunkData->locals.clear();
}

template <typename... Bytes>
void Compiler::emitBytes(u8 byte, Bytes ... rest) {
    emitByte(byte);
    emitBytes(rest...);
}

