#pragma once
#include "util.h"
#include "variant.h"
#include "compiler.h"

using Number = double;
using String = std::string;

using Value = Variant<
    Number,
    String,
    Shared<struct Module>
>;

struct Module {
    Chunk chunk;
    std::map<std::string, Value> globals;
};
