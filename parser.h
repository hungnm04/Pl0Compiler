#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "semantic.h"

void Program();
void block(const char* currentProcedureName);
void statement();

SymbolType condition();
SymbolType expression();
SymbolType term();
SymbolType factor();

void error(const char *message);

#endif
