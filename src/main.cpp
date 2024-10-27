#include <iostream>
#include <fstream>
#include <sstream>
#include "debug.h"
#include "timer.h"
#include "state.h"
#include "interpreter.h"
#include "print.h"

bool input(Interpreter* vm, Value* sp, int argc) {
    if (argc != 1) {
        return false;
    }

    if (sp[1].which() != Value::which<String>()) {
        return false;
    }

    String str = sp[1].get<String>();
    String val;
    std::cout << str;
    std::cin >> val;
    
    sp[0] = val;
    return true;
}

#include <random>

bool random(Interpreter* vm, Value* sp, int argc) {
    if (argc != 2) {
        return false;
    }

    if (sp[1].which() != Value::which<Number>() && sp[2].which() != Value::which<Number>()) {
        return false;
    }

    Number min = sp[1].get<Number>(); 
    Number max = sp[2].get<Number>(); 

    static std::random_device device;
    static std::mt19937 generator(device());
    std::uniform_int_distribution<> distr(min, max);

    sp[0] = (Number) distr(generator);
    return true;

}

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

    State state;
    state.base->globals["input"] = std::make_shared<BuiltInFunction>("input", &input);
    state.base->globals["random"] = std::make_shared<BuiltInFunction>("random", &random);
    
    clock.tick();
    state.run(source);
    clock.tock();

    print("Finished executing in", clock.duration().count(), "miliseconds");
}

void repl() {
    State state;
    state.base->globals["input"] = std::make_shared<BuiltInFunction>("input", &input);
    state.base->globals["random"] = std::make_shared<BuiltInFunction>("random", &random);

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
    runFile("test/code.jake");
    // repl();
    return 0;
}