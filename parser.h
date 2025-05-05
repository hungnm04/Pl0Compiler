#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"

void Program();
void block();
void statement();
void condition();  
void expression(); 
void term();      
void factor();     

void error(const char *message);

#endif




