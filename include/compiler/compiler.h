#pragma once
#include "compiler/bytecode.h"
#include "syntax/ast.h"
#include "syntax/scanner.h"
#include "util.h"
#include "error.h"

struct Prototype;

struct ConstantPool {
    std::vector<double> numbers;
    std::vector<std::string> names;
    std::vector<Prototype> prototypes;
};

struct Chunk {
    std::vector<u8> bytecode;
    std::vector<std::pair<int, SourceView>> markers;
    ConstantPool constants;
};

struct Prototype {
    std::string name;
    u8 argc;
    u8 upValues;
    Chunk chunk;
};

struct Local {
    std::string name;
    int depth;
};

struct UpValueData {
    u8 index;
    bool isLocal;
};

struct LoopData;
struct ChunkData;

struct ChunkData {
    int scopeDepth;
    int localOffset;
    bool global;
    Chunk chunk;
    std::vector<Local> locals;
    std::vector<UpValueData> upValues;
    std::unique_ptr<LoopData> loopData;
    std::unique_ptr<ChunkData> enclosing;
};

struct LoopData {
    std::unique_ptr<LoopData> enclosing;
    int start;
    std::vector<int> breaks;
};

class Compiler {
public:
    Compiler(std::string& path) : path(path) {};

    Chunk compile(Ast& ast);
    bool failed();
    Error getError();

private:
    // Chunk
    Chunk* getChunk();
    void newChunk();
    Chunk endChunk();

    // Scope
    void beginScope();
    void endScope();

    // Loop
    void beginLoop();
    void endLoop();

    // Error
    void errorAt(SourceView token, std::string msg, std::string note="");
    void internalError(std::string msg);

    // Constants
    int makeNumberConstant(double value, SourceView view);
    int makeNameConstant(std::string value, SourceView view);

    // Locals
    void addLocal(std::string name, SourceView view);
    int addUpValue(std::unique_ptr<ChunkData>& chunk, u8 index, bool isLocal, SourceView view);
    int findLocal(std::unique_ptr<ChunkData>& chunk, std::string name);
    int findUpValue(std::unique_ptr<ChunkData>& chunk, std::string name, SourceView view);

    // Variables
    void declare(std::string name, SourceView view);

    // Statements
    void body(std::vector<Stmt>& stmts);
    void breakStmt(BreakStmt& stmt);
    void continueStmt(ContinueStmt& stmt);
    void exitStmt(ExitStmt& stmt);
    void returnStmt(Ptr<ReturnStmt> stmt);
    void printStmt(Ptr<PrintStmt>& stmt);
    void ifStmt(Ptr<IfStmt>& stmt);
    void loopBlock(Ptr<LoopBlock>& stmt);
    void whileLoop(Ptr<WhileLoop>& stmt);
    void forLoop(Ptr<ForLoop>& stmt);
    void typeDeclaration(Ptr<TypeDeclaration>& stmt);
    void funcDeclaration(Ptr<FuncDeclaration>& stmt);
    void varDeclaration(Ptr<VarDeclaration>& stmt);

    // Expressions
    void expression(Expr expr);
    void assignment(Ptr<AssignmentExpr>& assignment);
    void identifier(Identifier& id, bool get);

    // Emit Byte
    void emitByte(u8 value);
    void emitByte(u16 value);
    template <typename First>
    void emitByte(First value);
    template <typename First, typename... Rest>
    void emitByte(First byte, Rest... rest);

    // Marker
    void marker(SourceView& view);

    // Emit Jump
    void emitJumpBackwards(u8 jump, int where);
    int emitJumpForwards(u8 jump);
    void patchJump(int index);

    bool hadError;
    Error error;
    std::string& path;
    std::unique_ptr<ChunkData> chunkData;
};
