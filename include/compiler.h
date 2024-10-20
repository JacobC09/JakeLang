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

struct UpValue {
    u8 index;
    bool inEnclosingChunk;
};

struct ChunkData {
    std::unique_ptr<struct ChunkData> enclosing;
    int scopeDepth;
    bool global;
    Chunk chunk;
    std::vector<Local> locals;
    std::vector<UpValue> upvalues;
};

struct LoopData {
    std::unique_ptr<struct LoopData> enclosing;
    int start;
    std::vector<int> breaks;
};

class Compiler {
public:
    Compiler() = default;

    Chunk compile(Ast& ast);
    bool failed();

private:
    Chunk* getChunk();
    void newChunk();
    Chunk endChunk();
    void newLoop();
    void endLoop();
    void error(std::string msg);
    int makeNumberConstant(double value);
    int makeNameConstant(std::string value);
    void addLocal(std::string name);
    int addUpValue(std::unique_ptr<ChunkData>& chunk, u8 index, bool inEnclosingChunk);
    int findLocal(std::unique_ptr<ChunkData>& chunk, std::string name);
    int findUpValue(std::unique_ptr<ChunkData>& chunk, std::string name);
    void beginScope();
    void endScope();
    void body(std::vector<Stmt>& stmts);
    void assignment(Ptr<AssignmentExpr>& assignment);
    void printStmt(Ptr<PrintStmt>& stmt);
    void ifStmt(Ptr<IfStmt>& stmt);
    void loopBlock(Ptr<LoopBlock>& stmt);
    void whileLoop(Ptr<WhileLoop>& stmt);
    void forLoop(Ptr<ForLoop>& stmt);
    void breakStmt();
    void continueStmt();
    void funcDeclaration(Ptr<FuncDeclaration>& stmt);
    void varDeclaration(Ptr<VarDeclaration>& stmt);
    void expression(Expr expr);
    void identifier(Identifier& id);

    void emitByte(u8 value);
    void emitByte(u16 value);
    template<typename First>
    void emitByte(First value);
    template<typename First, typename ... Rest>
    void emitByte(First byte, Rest ... rest);
    
    void emitJumpBackwards(u8 jump, int where);
    int emitJumpForwards(u8 jump);
    void patchJump(int index);

    std::unique_ptr<ChunkData> chunkData;
    std::unique_ptr<LoopData> loopData;

    bool hadError;
};

enum Instructions : u8 {
    OpPop,
    OpReturn,
    OpConstantName,
    OpConstantNumber,
    OpByteNumber,
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
    OpJump,
    OpJumpBack,
    OpJumpPopIfFalse,
    OpFunction,
    OpGetUpValue,
    OpSetUpValue,
    OpCloseUpValue,
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
