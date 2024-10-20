#include "state.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "print.h"
#include "value.h"

State::State() {};

Result State::run(std::string source) {
    Parser parser = Parser(source);
    Ast ast = parser.parse();

    if (parser.failed())
        return Result{ExitCode::Failed};

    printAst(ast);

    Compiler compiler;
    Chunk chunk = compiler.compile(ast);

    if (compiler.failed()) {
        return Result { ExitCode::Failed };
    }

    printChunk(chunk);

    Shared<Module> mod = std::make_shared<Module>(chunk);

    return Result{ExitCode::Success};
}
Result State::run(Module mod) {
    return Result();
};