#include "state.h"

#include "builtins.h"
#include "compiler/compiler.h"
#include "interpreter/interpreter.h"
#include "print.h"
#include "syntax/ast.h"
#include "syntax/parser.h"

State::State() {
    base = std::make_shared<Module>();
    base->name = "base";

    Shared<Module> mod = initBuiltins();
    base->globals.merge(mod->globals);
}

Result State::run(std::string source) {
    Parser parser = Parser(source, base->name);
    Ast ast = parser.parse();


    if (parser.failed()) {
        printError(parser.getError(), source);
        return Result{ExitCode::Failed};
    }

    Compiler compiler = Compiler(base->name);
    Chunk chunk = compiler.compile(ast);

    if (compiler.failed()) {
        printError(compiler.getError(), source);
        return Result{ExitCode::Failed};
    }

    Interpreter interpreter = Interpreter(*this);
    Result res = interpreter.interpret(base, chunk);

    if (res.exitCode > 0) {
        printError(interpreter.error, source);
    }

    return res;
}
