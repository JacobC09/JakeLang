#pragma once
#include "compiler/compiler.h"
#include "util.h"
#include "variant.h"

class BuiltInHelper;

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
    Shared<struct Module>>;

using BuiltInFunctionPtr = void (*)(BuiltInHelper helper, int argc);

struct BuiltInFunction {
    std::string name;
    BuiltInFunctionPtr ptr;
};

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
