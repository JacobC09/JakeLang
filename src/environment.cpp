#include "environment.h"
#include "ast.h"
#include "parser.h"
#include "print.h"

Environment::Environment() {};

Result Environment::run(std::string source) {
    Parser parser = Parser(source);
    Ast ast = parser.parse();

    if (parser.failedParse()) 
        return Result { ExitCode::Failed };
        
    printAst(ast);
    
    return Result { ExitCode::Success };
};