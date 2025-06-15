#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "semantic.h"
#include "codegen.h"

typedef struct {
    int isConstant;
    int value;
} ExpressionResult;

void Program();
void block();
void statement();
void condition();

ExpressionResult expression();
ExpressionResult term();
ExpressionResult factor();

void error(const char *message);

#endif