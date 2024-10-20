#include <iostream>
#include <fstream>
#include <sstream>
#include "debug.h"
#include "timer.h"
#include "state.h"

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

    print(">=== Source ===<");
    print(source);
    print(">==============<");

    clock.tick();

    State state;
    state.run(source);

    clock.tock();

    print("Finished executing in", clock.duration().count(), "miliseconds");
}

int main(int argc, const char* argv[]) {
    std::string myString = "Hello";
    const char* myCStr = "World";
    int myHex = 255;
    std::string formatted = formatStr("Int: %05d, Float: %.2f, String: %s, Char: %c, CStr: %p, Hex: %04x", 42, 3.14159, myString, 'A', myCStr, myHex);
    std::cout << formatted << std::endl;

    // runFile("test/code.jake");
    return 0;
}