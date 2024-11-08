#include "builtins.h"

#include <random>

#include "print.h"

void builtIns::input(BuiltInHelper helper, int argc) {
    if (helper.assertArgc(argc, 1)) return;
    if (helper.assertArgType(0, Value::which<String>())) return;

    std::cout << helper.arg(0).get<String>();
    std::string returnVal;
    std::getline(std::cin, returnVal);
    helper.setReturn(returnVal);
}

void builtIns::random(BuiltInHelper helper, int argc) {
    if (helper.assertArgc(argc, 2)) return;
    if (helper.assertArgType(0, Value::which<Number>())) return;
    if (helper.assertArgType(1, Value::which<Number>())) return;

    static std::random_device device;
    static std::mt19937 generator(device());

    Number min = helper.arg(0).get<Number>();
    Number max = helper.arg(1).get<Number>();

    std::uniform_int_distribution<> distr(min, max);

    helper.setReturn((Number)distr(generator));
}

void BuiltInHelper::setReturn(Value value) {
    stack[0] = value;
}

void BuiltInHelper::error(std::string msg) {
    interpreter->errorAt(msg);
}

bool BuiltInHelper::assertArgc(int argc, int expected) {
    if (argc != expected) {
        error(formatStr("Expected %d argument%p, got %d", expected, expected > 1 ? "s" : "", argc));
        return true;
    }
    return false;
}

bool BuiltInHelper::assertArgType(int index, int type) {
    int argType = arg(index).which();
    if (argType != type) {
        error(formatStr("Expected argument %d to be of type '%s', got '%s' instead", index, getTypename(type), getTypename(argType)));
        return true;
    }
    return false;
}

Value BuiltInHelper::arg(int index) {
    return stack[index + 1];
}

Shared<Module> initBuiltins() {
    auto mod = std::make_shared<Module>();
    std::vector<std::pair<std::string, BuiltInFunctionPtr>> pairs = {
        {"input", &builtIns::input},
        {"random", &builtIns::random},
    };

    for (auto& [name, ptr] : pairs) {
        Shared<BuiltInFunction> func = std::make_shared<BuiltInFunction>();
        func->name = name;
        func->ptr = ptr;
        mod->globals[name] = func;
    }

    return mod;
}
