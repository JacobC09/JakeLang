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
    Shared<struct UpValue>,
    Shared<struct Function>,
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

struct Module {
    std::string name;
    std::map<std::string, Value> globals;
};
