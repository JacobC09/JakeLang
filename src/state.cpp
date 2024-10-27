#include "state.h"

#include "builtins.h"
#include "compiler/compiler.h"
#include "interpreter/interpreter.h"
#include "print.h"
#include "syntax/ast.h"
#include "syntax/parser.h"

State::State() {
    base = std::make_shared<Module>();

    Shared<Module> mod = initBuiltins();
    base->globals["__builtIns__"] = mod;
    base->globals.merge(mod->globals);
}

Result State::run(std::string source) {
    Parser parser = Parser(source);
    Ast ast = parser.parse();

    if (parser.failed())
        return Result{ExitCode::Failed};

    Compiler compiler;
    Chunk chunk = compiler.compile(ast);

    if (compiler.failed()) {
        return Result{ExitCode::Failed};
    }

    Interpreter interpreter = Interpreter(*this);

    return interpreter.interpret(base, chunk);
}

// Result State::run(std::string source) {
//     Parser parser = Parser(source);
//     Ast ast = parser.parse();

//     if (parser.failed())
//         return Result{ExitCode::Failed};

//     printAst(ast);

//     Compiler compiler;
//     Chunk chunk = compiler.compile(ast);

//     if (compiler.failed()) {
//         return Result { ExitCode::Failed };
//     }

//     printChunk(chunk);

//     Interpreter interpreter = Interpreter(*this);
//     print(">=== Output ===<");
//     Result result = interpreter.interpret(base, chunk);
//     print(">==============<");
//     return result;
// }
