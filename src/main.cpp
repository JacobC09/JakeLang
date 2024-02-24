#include <iostream>
#include <fstream>
#include <sstream>
#include "debug.h"
#include "timer.h"
#include "environment.h"

std::string openFile(std::string path) {
    std::fstream file;

    file.open(path);
    if (!file.is_open()) {
        print("[Error] Failed to open source file");
        return std::string{};
    }

    std::stringstream stream;
    stream << file.rdbuf();
    file.close();

    return stream.str();
}

void runFile(std::string path) {
    std::string source = openFile(path);
    Timer<std::chrono::microseconds> clock;

    print(source);

    clock.tick();

    Environment env;
    env.run(source);

    clock.tock();

    print("Finished executing in", clock.duration().count(), "miliseconds");
}

int main(int argc, const char* argv[]) {
    runFile("test/code.jake");
    return 0;
}