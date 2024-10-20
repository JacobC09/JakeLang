#include "value.h"

struct CallFrame {
    Module mod;
    Chunk code;
    Function func;  // nullptr if in global scope
};