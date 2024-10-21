#include "value.h"
#include "state.h"

const int frames_max = 64;
const int stack_max = frames_max * UINT8_MAX;

struct CallFrame {
    u8* ip;
    Value* sp;
    Module& mod;
    Chunk& code;
    Function* func;
};

class Interpreter {
public:
    Interpreter(State& state);

    Result interpret(Module& mod, Chunk& chunk);
    Result run();

private:
    void error(std::string msg);
    int pc();
    u8 readByte();
    u16 readShort();
    Number readNumberConstant();
    String readNameConstant();
    void push(Value value);
    Value pop();
    Value peek(int offset);
    CallFrame* frame();

    bool valuesEqual(Value& a, Value& b);
    bool isTruthy(Value& value);

    bool hadError;
    State& state;
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
};