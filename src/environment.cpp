#include "environment.h"
#include "ast.h"

Environment::Environment() {};

Result Environment::run(std::string source) {
    return Result { Success };
};