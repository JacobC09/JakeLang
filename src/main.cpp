#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <chrono>
#include <memory>
#include "debug.h"
#include "timer.h"
#include "environment.h"
#include "varient.h"

void runFile(const char* path) {
    std::fstream file;

    file.open(path);
    if (!file.is_open()) {
        print("[Error] Failed to open source file");
        return;
    }

    std::stringstream stream;
    stream << file.rdbuf();
    std::string source = stream.str();

    file.close();

    Timer<std::chrono::microseconds> clock;

    clock.tick();

    Environment env;
    env.run(source);

    clock.tock();

    print("Finished executing in", clock.duration().count(), "miliseconds");
}

int main(int argc, const char* argv[]) {

    return 0;
}