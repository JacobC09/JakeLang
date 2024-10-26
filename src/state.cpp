#include "state.h"
#include "ast.h"
#include "compiler.h"
#include "parser.h"
#include "print.h"
#include "value.h"
#include "interpreter.h"

State::State() {
    base = std::make_shared<Module>();
}

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

    Interpreter interpreter = Interpreter(*this);
    print(">=== Output ===<");
    Result result = interpreter.interpret(base, chunk);
    print(">==============<");
    return result;
}
