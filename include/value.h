#pragma once
#include "util.h"
#include "variant.h"
#include "compiler.h"
// #include "interpreter.h"

struct None {};

using Number = double;
using String = std::string;
using Boolean = bool;

using Value = Variant<
    None,
    Number,
    String,
    Boolean,
    Shared<struct UpValue>,
    Shared<struct Function>,
    Shared<struct BuiltInFunction>,
    Shared<struct Module>
>;

struct UpValue {
    Value* loc;
    Value owned;
    Shared<struct UpValue> next;
};

struct Function {
    Prototype prot;
    Shared<Module> mod;
    std::vector<Shared<UpValue>> upValues;
};

using BuiltInFunctionPtr = bool(*)(class Interpreter*, Value* sp, int argc);

struct BuiltInFunction {
    BuiltInFunction() = default;
    BuiltInFunction(std::string name, BuiltInFunctionPtr func) : name(name), func(func) {}

    std::string name;
    BuiltInFunctionPtr func;
};

struct Module {
    std::string name;
    std::map<std::string, Value> globals;
};
