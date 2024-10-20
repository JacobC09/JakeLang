#include "interpreter.h"
#include "print.h"

Interpreter::Interpreter(State& state) : state(state) {
    stack.reserve(stack_max);
    frames.reserve(frames_max);
}

Result Interpreter::interpret(Module& mod, Chunk& chunk) {
    frames.push_back(CallFrame{chunk.bytecode.data(), stack.data(), mod, chunk, nullptr});

    hadError = false;
    return run();
}

Result Interpreter::run() {
    for (;;) {
        u8 instruction = readByte();

        switch (instruction) {
            case OpPop: {
                stack.resize(stack.size() - readByte());
                break;
            }

            case OpReturn: {
                Value val = pop();
                return Result {0};
            };


            case OpConstantName: {
                push(readNameConstant());
                break;
            }

            case OpConstantNumber: {
                push(readNumberConstant());
                break;
            }

            case OpByteNumber: {
                push((double) readByte());
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
                Value a = pop();
                Value b = pop();

                if (a.is<Number>() && b.is<Number>()) {
                    push(a.get<Number>() + b.get<Number>());
                } else if (a.is<String>() && b.is<String>()) {
                    push(a.get<String>() + b.get<String>());
                } else {
                    error("Can only add numbers or strings");
                    return Result {1};
                }

                break;
            }

            case OpSubtract: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only subtract numbers");
                    return Result {1};
                }

                push(a.get<Number>() > b.get<Number>());
                break;
            }

            case OpMultiply: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only multiply numbers");
                    return Result {1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpDivide: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only divide numbers");
                    return Result {1};
                }

                push(a.get<Number>() / b.get<Number>());
                break;
            }

            case OpEqual: {
                Value a = pop();
                Value b = pop();
                push(valuesEqual(a, b));

                break;
            }

            case OpGreater: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result {1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpLess: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result {1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpGreaterThanOrEq: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result {1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpLessThanOrEq: {
                Value a = pop();
                Value b = pop();

                if (!a.is<Number>() || !b.is<Number>()) {
                    error("Can only compare numbers");
                    return Result {1};
                }

                push(a.get<Number>() * b.get<Number>());
                break;
            }

            case OpNegate: {
                Value a = pop();

                if (!a.is<Number>()) {
                    error("Can only negate a number");
                    return Result {1};
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
                frame()->mod.globals[readNameConstant()] = pop();
                break;
            }

            case OpGetGlobal: {
                String name = readNameConstant();
                auto value = frame()->mod.globals.find(name);

                if (value == frame()->mod.globals.end()) {
                    error(formatStr("Couldn't find global named %s in current module", name));
                    return Result {1};
                }

                push(value->second);
                break;
            }

            case OpSetGlobal: {
                String name = readNameConstant();
                auto value = frame()->mod.globals.find(name);

                if (value == frame()->mod.globals.end()) {
                    error(formatStr("Couldn't find global named %s in current module", name));
                    return Result {1};
                }

                value->second = peek(0);
                break;
            };

            case OpGetLocal: {
                push(frame()->sp[readByte()]);
                break;
            }

            case OpSetLocal: {
                frame()->sp[readByte()] = peek(0);
                break;
            }

            case OpJump: {
                frame()->ip += readShort();
                break;
            }

            case OpJumpBack: {
                frame()->ip -= readShort();
                break;
            }

            case OpJumpPopIfFalse: {
                u16 distance = readShort();
                frame()->ip += !isTruthy(pop()) * distance;
                break;
            }

            default: {
                error(formatStr("Unknown Instruction (%d)", (int)instruction));
                return Result {1};
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

u8 Interpreter::readByte() {
    return *frame()->ip++;
}

u16 Interpreter::readShort() {
    frame()->ip += 2;
    return (u16)((frame()->ip[-1] << 8) | frame()->ip[-2]);
}

Number Interpreter::readNumberConstant() {
    return frame()->code.constants.numbers[readByte()];
}

String Interpreter::readNameConstant() {
    return frame()->code.constants.names[readByte()];
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

CallFrame* Interpreter::frame() {
    return &frames.back();
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
        case Value::which<Shared<Upvalue>>(): {
            return isTruthy(*value.get<Shared<Upvalue>>()->loc);
        }
        default:
            return true;
    }
}
