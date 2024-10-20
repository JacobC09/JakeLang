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
    State() = default;

    Result run(std::string source);

    // Module loadModule(std::string name);

private:
    Module base;
};