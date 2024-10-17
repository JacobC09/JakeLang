#pragma once
#include "ast.h"
#include "util.h"

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

struct Local {
    std::string name;
    int depth;
};

struct ChunkData {
    ChunkData() = default;
    int scopeDepth;
    bool global;
    Chunk chunk;
    std::vector<Local> locals;
    std::unique_ptr<struct ChunkData> enclosing;
};

class Compiler {
public:
    Compiler() = default;

    Chunk compile(Ast& ast);
    bool failed();

private:
    Chunk* getChunk();
    void error(std::string msg);
    void newChunk();
    void endChunk();
    int makeNumberConstant(double value);
    int makeNameConstant(std::string value);
    void addLocal(std::string name);
    int findLocal(std::string name);
    void beginScope();
    void endScope();
    void body(std::vector<Stmt>& stmts);
    void expression(Expr expr);
    void assignment(Ptr<AssignmentExpr>& assignment);
    void identifier(Identifier& id);
    void printStmt(Ptr<PrintStmt>& stmt);
    void ifStmt(Ptr<IfStmt>& stmt);
    void loopBlock(Ptr<LoopBlock>& stmt);
    void whileLoop(Ptr<WhileLoop>& stmt);
    void forLoop(Ptr<ForLoop>& stmt);
    void funcDeclaration(Ptr<FuncDeclaration>& stmt);
    void varDeclaration(Ptr<VarDeclaration>& stmt);

    void emitByte(u8 value);
    void emitByte(u16 value);
    template<typename First>
    void emitByte(First value);
    template<typename First, typename ... Rest>
    void emitByte(First byte, Rest ... rest);
    
    int emitJump(u8 jump);
    void patchJump(int index);

    std::unique_ptr<ChunkData> chunkData;

    bool hadError;
};

enum Instructions : u8 {
    OpPop,
    OpReturn,
    OpConstantName,
    OpConstantNumber,
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
