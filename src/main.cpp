#include <fstream>
#include <iostream>
#include <sstream>

#include "debug.h"
#include "interpreter/interpreter.h"
#include "print.h"
#include "state.h"
#include "timer.h"

std::string openFile(std::string path) {
    std::fstream file;
    file.open(path);
    if (!file.is_open()) {
        print("Failed to open source file");
        return std::string{};
    }

    std::stringstream stream;
    stream << file.rdbuf();
    file.close();

    return stream.str();
}

void runFile(std::string path) {
    State state;
    state.run(openFile(path));
}

void repl() {
    State state;
    std::string input;
    for (;;) {
        printf(">>> ");
        std::getline(std::cin, input);

        if (input == "exit") {
            break;
        }

        state.run(input);
    }
}

int main(int argc, const char* argv[]) {
    if (argc == 1) {
        runFile(argv[0]);
    } else {
        repl();
    }
}