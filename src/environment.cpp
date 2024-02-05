#include "environment.h"
#include "ast.h"
#include "parser.h"
#include "print.h"

Environment::Environment() {};

Result Environment::run(std::string source) {
    Parser parser = Parser(source);
    std::unique_ptr<Ast> ast = parser.parse();
    
    return Result { ExitCode::Success };
};