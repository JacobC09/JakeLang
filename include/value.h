#pragma once
#include "util.h"
#include "variant.h"
#include "compiler.h"

using Number = double;
using String = std::string;

using Value = Variant<
    Number,
    String,
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
    Module(Chunk chunk) : chunk(chunk) {}

    Chunk chunk;
    std::map<std::string, Value> globals;
};
