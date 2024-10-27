#pragma once
#include "compiler/compiler.h"
#include "interpreter/value.h"
#include "syntax/ast.h"
#include "syntax/scanner.h"

std::string getValueStr(const Value& value);
std::string getTypename(int which);

void printToken(const Token& token);
void printAst(const Ast& ast);
void printChunk(const Chunk& chunk, std::string name = "");
void printValue(const Value& value);