#pragma once
#include <string>
#include "syntax/token.h"

struct Error {
    SourceView view;
    std::string type;
    std::string msg;
    std::string note;
    std::string path;
};