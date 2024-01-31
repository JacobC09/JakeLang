#pragma once
#include <string>

enum ExitCode : int {
    Success,
    Failed
};

struct Result {
    int exitCode;
};

class Environment {
public:
    Environment();

    Result run(std::string source);
};