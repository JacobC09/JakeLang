#pragma once
#include "util.h"
#include "ast.h"
#include <set>

struct Prototype;

struct ConstantPool {
    std::vector<double> numbers;
    std::vector<std::string> names;
    std::vector<Prototype> prototypes;
};

struct Chunk {
    std::vector<u8> bytecode;
    ConstantPool constants;
};

struct Prototype {
    std::string name;
    int argc;
    Chunk chunk;
};

struct ChunkData {
    ChunkData() = default;

    Chunk chunk;
};

class Compiler {
public:
    Compiler() = default;

    Chunk compile(Ast& ast);
    bool failed();

private:
    Chunk* getChunk();
    void error(std::string msg);
    void emitByte(u8 byte);

    void body(std::vector<Stmt>& stmts);
    void expression(Expr expr);
    void emitNumberConstant(double value);
    void emitNameConstant(std::string value);

    std::unique_ptr<ChunkData> chunkData;

    bool hadError;
};


enum Instructions : u8 {
    OpPop,
    OpReturn,
    OpConstantName,
    OpConstantNumber,
    OpConstantNameDouble,
    OpConstantNumberDouble,
    OpTrue,
    OpFalse,
    OpNone,
    OpAdd,
    OpSubtract,
    OpMultiply,
    OpDivide,
    OpExponent,
    OpEqual,
    OpGreater,
    OpLess,
    OpGreaterThanOrEq,
    OpLessThanOrEq,
    OpNot,
    OpNegate,
    OpPrint,
    OpDefineGlobal,
    OpGetGlobal,
    OpSetGlobal,
    OpGetLocal,
    OpSetLocal,
    OpGetUpValue,
    OpSetUpValue,
    OpCloseUpValue,
    OpJump,
    OpJumpBack,
    OpJumpIfTrue,
    OpJumpIfFalse,
    OpCall,
    OpClosure,
    OpClass,
    OpGetProperty,
    OpSetProperty,
    OpMethod,
    OpInvoke,
    OpInherit,
    OpGetSuper
};
