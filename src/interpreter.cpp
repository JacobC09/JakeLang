#include "interpreter.h"
#include "print.h"

Interpreter::Interpreter(State& state) : state(state) {
    stack.reserve(stack_max);
    frames.reserve(frames_max);
}

Result Interpreter::interpret(Shared<Module> mod, Chunk& chunk) {
    newFrame(mod, chunk, stack.data(), nullptr);
    hadError = false;
    openUpValues = nullptr;
    return run();
}

Result Interpreter::run() {
    CallFrame* frame = getFrame();

    for (;;) {
        u8 instruction = readByte();

        switch (instruction) {
            case OpExit: {
                return Result{(int)readByte()};
            }

            case OpReturn: {
                closeUpValues(frame->sp);
                frames.pop_back();
                frame = getFrame();
                break;
            }

            case OpPop: {
                stack.pop_back();
                break;
            }

            case OpName: {
                push(readNameConstant());
                break;
            }

            case OpNumber: {
                push(readNumberConstant());
                break;
            }

            case OpByteNumber: {
                push((double)readByte());
                break;
            }

            case OpTrue: {
                stack.push_back(true);
                break;
            }

            case OpFalse: {
                push(false);
                break;
            }

            case OpNone: {
                push(None{});
                break;
            }

            case OpAdd: {
                Value b = pop();
                Value a = pop();

                if (a.is<Number>() && b.is<Number>()) {
                    push(a.get<Number>() + b.get<Number>());
                } else if (a.is<String>() && b.is<String>()) {
                    push(a.get<String>() + b.get<String>());
                } else {
                    error("Can only add numbers or strings");
                    return Result{1};
                }

                break;
            }

            case OpSubtract: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only subtract numbers");
                    return Result{1};
                }

                push(a.get<Number>() - b.get<Number>());
                break;
            }

            case OpModulous: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only modulous numbers");
                    return Result{1};
                }

                push(std::fmod(a.get<Number>(), b.get<Number>()));
                break;
            }

            case OpMultiply: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only multiply numbers");
                    return Result{1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpDivide: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only divide numbers");
                    return Result{1};
                }

                push(a.get<Number>() / b.get<Number>());
                break;
            }

            case OpEqual: {
                Value b = pop();
                Value a = pop();
                push(valuesEqual(a, b));
                break;
            }

            case OpGreater: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result{1};
                }

                push(a.get<Number>() > b.get<Number>());
                break;
            }

            case OpLess: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result{1};
                }

                push(a.get<Number>() < b.get<Number>());
                break;
            }

            case OpGreaterThanOrEq: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result{1};
                }

                push(a.get<Number>() >= b.get<Number>());
                break;
            }

            case OpLessThanOrEq: {
                Value b = pop();
                Value a = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result{1};
                }

                push(a.get<Number>() <= b.get<Number>());
                break;
            }

            case OpNegate: {
                Value a = pop();

                if (!a.is<Number>()) {
                    error("Can only negate a number");
                    return Result{1};
                }

                push(-a.get<Number>());
                break;
            }

            case OpNot: {
                push(!isTruthy(pop()));
                break;
            }

            case OpPrint: {
                std::string output;
                for (int count = readByte(); count > 0; count--) {
                    output += getValueStr(pop());
                    if (count > 1) {
                        output += " ";
                    }
                }
                print(output);
                break;
            }

            case OpDefineGlobal: {
                frame->mod->globals[readNameConstant()] = pop();
                break;
            }

            case OpGetGlobal: {
                String name = readNameConstant();
                auto value = frame->mod->globals.find(name);

                if (value == frame->mod->globals.end()) {
                    error(formatStr("Couldn't find global named '%s' in current module", name));
                    return Result{1};
                }

                push(value->second);
                break;
            }

            case OpSetGlobal: {
                String name = readNameConstant();
                auto value = frame->mod->globals.find(name);

                if (value == frame->mod->globals.end()) {
                    error(formatStr("Couldn't find global named '%s' in current module", name));
                    return Result{1};
                }

                value->second = peek(0);
                break;
            };

            case OpGetLocal: {
                push(frame->sp[readByte()]);
                break;
            }

            case OpSetLocal: {
                frame->sp[readByte()] = peek(0);
                break;
            }

            case OpGetProperty: {
                break;
            }

            case OpSetProperty: {
                break;
            }

            case OpGetUpValue: {
                push(*frame->func->upValues[readByte()]->loc);
                break;
            }

            case OpSetUpValue: {
                *frame->func->upValues[readByte()]->loc = peek(0);
                break;
            }

            case OpPopLocals: {
                u8 amount = readByte();
                closeUpValues(stack.data() + stack.size() - amount);
                stack.resize(stack.size() - amount);
                break;
            }

            case OpJump: {
                frame->ip += readShort();
                break;
            }

            case OpJumpBack: {
                frame->ip -= readShort();
                break;
            }

            case OpJumpIfTrue: {
                u16 distance = readShort();
                frame->ip += isTruthy(peek(0)) * distance;
                break;
            }

            case OpJumpIfFalse: {
                u16 distance = readShort();
                frame->ip += !isTruthy(peek(0)) * distance;
                break;
            }

            case OpJumpPopIfFalse: {
                u16 distance = readShort();
                frame->ip += !isTruthy(pop()) * distance;
                break;
            }

            case OpFunction: {
                Shared<Function> func = std::make_shared<Function>();
                func->mod = frame->mod;
                func->prot = frame->chunk.constants.prototypes[readByte()];

                for (int i = 0; i < func->prot.upValues; i++) {
                    u8 index = readByte();
                    u8 isLocal = readByte();
                    if (isLocal) {
                        func->upValues.push_back(captureUpValue(frame->sp + index));
                    } else {
                        func->upValues.push_back(frame->func->upValues[index]);
                    }
                }

                push(func);
                break;
            };

            case OpCall: {
                bool success = callValue(pop());
                if (!success) {
                    return Result{1};
                }
                frame = getFrame();
                break;
            }

            default: {
                error(formatStr("Unknown Instruction (%d)", (int)instruction));
                return Result{1};
            }
        }
    }
}

void Interpreter::error(std::string msg) {
    if (hadError) return;

    hadError = true;
    printf("Error during execution\n");
    printf("    %s\n", msg.c_str());
}

int Interpreter::pc() {
    return getFrame()->ip - getFrame()->chunk.bytecode.data() - 1;
}

u8 Interpreter::readByte() {
    return *getFrame()->ip++;
}

u16 Interpreter::readShort() {
    getFrame()->ip += 2;
    return (u16)((getFrame()->ip[-2] << 8) | getFrame()->ip[-1]);
}

Number Interpreter::readNumberConstant() {
    return getFrame()->chunk.constants.numbers[readByte()];
}

String Interpreter::readNameConstant() {
    return getFrame()->chunk.constants.names[readByte()];
}

void Interpreter::push(Value value) {
    stack.push_back(value);
}

Value Interpreter::pop() {
    if (stack.size()) {
        auto val = stack.back();
        stack.pop_back();
        return val;
    }

    error("Tried to pop on empty stack");
    return None{};
}

Value Interpreter::peek(int offset) {
    return stack[stack.size() - offset - 1];
}

bool Interpreter::callValue(Value value) {
    u8 argc = readByte();
    Value* sp = stack.data() + stack.size() - argc - 1;

    switch (value.which()) {
        case Value::which<Shared<Function>>(): {
            auto func = value.get<Shared<Function>>();
            if (argc != func->prot.argc) {
                error(formatStr("Expected %d argument%p, got %d", func->prot.argc, func->prot.argc > 1 ? "s" : "", argc));
                return false;
            }

            newFrame(func->mod, func->prot.chunk, sp, func);
            return true;
        }

        case Value::which<Shared<BuiltInFunction>>(): {
            int stackSize = stack.size();
            bool result = value.get<Shared<BuiltInFunction>>()->func(this, sp, argc);
            if (argc) {
                stack.resize(stackSize - argc);
            }
            return result;
        }

        default:
            error("Invalid call target");
            return false;
    }
}

CallFrame* Interpreter::getFrame() {
    return &frames.back();
}

void Interpreter::newFrame(Shared<Module> mod, Chunk& chunk, Value* sp, Shared<Function> func) {
    frames.push_back(CallFrame{
        CallFrame{chunk.bytecode.data(), sp, mod, chunk, func}});
}

Shared<UpValue> Interpreter::captureUpValue(Value* local) {
    UpValue* prev = nullptr;
    Shared<UpValue> current = openUpValues;

    while (current != nullptr && current->loc > local) {
        prev = current.get();
        current = current->next;
    }

    if (current != nullptr && current->loc == local) {
        return current;
    }

    Shared<UpValue> upValue = std::make_shared<UpValue>();
    upValue->loc = local;
    upValue->next = current;

    if (prev == nullptr) {
        openUpValues = upValue;
    } else {
        prev->next = upValue;
    }

    return upValue;
}

void Interpreter::closeUpValues(Value* minLoc) {
    while (openUpValues != nullptr && openUpValues->loc >= minLoc) {
        openUpValues->owned = *openUpValues->loc;
        openUpValues->loc = &openUpValues->owned;
        openUpValues = openUpValues->next;
    }
}

void Interpreter::printStack() {
    print(">=== Stack ===<");
    for (int i = 0; i < (signed)stack.size(); i++) {
        printf("%d: %s\n", i, getValueStr(stack[i]).c_str());
    }
    print(">=============<");
}

bool Interpreter::valuesEqual(Value& a, Value& b) {
    if (a.which() != b.which()) {
        return false;
    }

    if (a.is<Number>() && b.is<Number>()) {
        return a.get<Number>() == b.get<Number>();
    }

    if (a.is<String>() && b.is<String>()) {
        return a.get<String>() == b.get<String>();
    }

    if (a.is<Boolean>()) {
        return isTruthy(b) == a.get<Boolean>();
    }

    if (b.is<Boolean>()) {
        return isTruthy(a) == b.get<Boolean>();
    }

    if (a.is<None>() && b.is<None>()) {
        return true;
    }

    return false;
}

bool Interpreter::isTruthy(Value& value) {
    switch (value.which()) {
        case Value::which<Number>(): {
            return value.get<Number>() != 0;
        }
        case Value::which<String>(): {
            return value.get<String>().size();
        }
        case Value::which<Boolean>(): {
            return value.get<bool>();
        }
        case Value::which<None>(): {
            return false;
        }
        case Value::which<Shared<UpValue>>(): {
            return isTruthy(*value.get<Shared<UpValue>>()->loc);
        }
        default:
            return true;
    }
}
