#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENT_LEN 32
typedef enum {
 NONE=0, IDENT, NUMBER,
 BEGIN, CALL, CONST, DO,  ELSE, END, FOR, IF, ODD,
 PROCEDURE, PROGRAM, THEN, TO, VAR, WHILE,
 PLUS, MINUS, TIMES, SLASH, EQU, NEQ, LSS, LEQ, 
GTR, GEQ, LPARENT, RPARENT, LBRACK, RBRACK, 
PERIOD, COMMA, SEMICOLON,  ASSIGN, PERCENT
} TokenType;

extern TokenType Token;  
extern int Num;              
extern char Id[MAX_IDENT_LEN+1]; //string when Token is ident
extern char Ch;              // current character
extern FILE* input;         

void initLexer(const char* input);
void closeLexer();
TokenType getToken();
void printToken();
const char* getTokenName(TokenType token);



