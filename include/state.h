#pragma once
#include <string>

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
};