#pragma once
#include "util.h"
#include "variant.h"
#include "compiler.h"

using Number = double;
using String = std::string;
using Boolean = bool;

struct None {};

using Value = Variant<
    Number,
    String,
    Boolean,
    None,
    Shared<struct Upvalue>,
    Shared<struct Function>,
    Shared<struct Module>
>;

struct Upvalue {
    Value* loc;
    Value ownedValue;
};

struct Function {
    Prototype prot;
    std::vector<Upvalue> upvalues;
};

struct Module {
    std::string name;
    std::map<std::string, Value> globals;
};
