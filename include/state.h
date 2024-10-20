#pragma once
#include <string>
#include "value.h"

enum ExitCode : int {
    Success,
    Failed
};

struct Result {
    int exitCode;
};

class State {
public:
    State();

    Result run(std::string source);
    Result run(Module mod);
};