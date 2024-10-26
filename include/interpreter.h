#include "value.h"
#include "state.h"

const int frames_max = 64;
const int stack_max = frames_max * UINT8_MAX;

struct CallFrame {
    u8* ip;
    Value* sp;
    Shared<Module> mod;
    Chunk& chunk;
    Shared<Function> func;
};

class Interpreter {
public:
    Interpreter(State& state);

    Result interpret(Shared<Module> mod, Chunk& chunk);
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
    CallFrame* getFrame();
    void newFrame(Shared<Module> mod, Chunk& chunk, Value* sp, Shared<Function> func);
    Shared<UpValue> captureUpValue(Value* local);
    void closeUpValues(Value* minLoc);
    void printStack();

    bool valuesEqual(Value& a, Value& b);
    bool isTruthy(Value& value);

    bool hadError;
    State& state;

    Shared<UpValue> openUpValues;
    std::vector<Value> stack;
    std::vector<CallFrame> frames;
};