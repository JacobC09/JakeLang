#pragma once
#include "scanner.h"
#include "ast.h"
#include "value.h"

void printToken(const Token& token);
void printAst(const Ast& ast);
void printChunk(const Chunk& chunk, std::string name="");