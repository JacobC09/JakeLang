#pragma once
#include "interpreter/interpreter.h"
#include "interpreter/value.h"

class BuiltInHelper {
public:
    Interpreter* interpreter;
    Value* stack;

    BuiltInHelper(Interpreter* interpreter, Value* stack) : interpreter(interpreter), stack(stack) {};

    void setReturn(Value value);
    void error(std::string msg);
    bool assertArgc(int argc, int expected);
    bool assertArgType(int index, int type);
    Value arg(int index);
};

Shared<Module> initBuiltins();

namespace builtIns {
void input(BuiltInHelper helper, int argc);
void random(BuiltInHelper helper, int argc);
}  // namespace builtIns
