#pragma once
#include "scanner.h"
#include "ast.h"
#include "compiler.h"
#include "value.h"

std::string getValueStr(const Value& value);
std::string getTypename(int which);

void printToken(const Token& token);
void printAst(const Ast& ast);
void printChunk(const Chunk& chunk, std::string name="");
void printValue(const Value& value);