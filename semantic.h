#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "lexer.h"

#define MAX_PARAMS 10

typedef enum {
    SYM_CONST, SYM_VAR, SYM_ARRAY, SYM_PROCEDURE,
    SYM_PARAM_VAL, SYM_PARAM_VAR
} SymbolType;

typedef struct ParamNode {
    char name[MAX_IDENT_LEN+1];
    int isVar;
    struct ParamNode* next;
} ParamNode;

typedef struct SymbolNode {
    char name[MAX_IDENT_LEN+1];
    SymbolType type;
    int level;
    int address;
    union {
        int value;
        struct { int size; } array;
        struct {
            ParamNode* params;
            int paramCount;
        } proc;
    } info;
    struct SymbolNode* next;
} SymbolNode;

typedef struct Scope {
    SymbolNode* symbols;
    struct Scope* parent;
    char procName[MAX_IDENT_LEN+1];
    int dataSize;
} Scope;

void initSymbolTable();
void freeSymbolTable();
void enterScope(const char* procName);
void exitScope();
void addConstant(const char* name, int value);
void addVariable(const char* name, int level, int* address);
void addArray(const char* name, int size, int level, int* address);
SymbolNode* addProcedure(const char* name, int level, int address);
void addParameter(SymbolNode* procSymbol, const char* paramName, int isVar, int level, int* address);
SymbolNode* findSymbol(const char* name);
SymbolNode* findSymbolInCurrentScope(const char* name);
int isDeclaredInCurrentScope(const char* name);

extern int currentLevel;
extern Scope* currentScope;
void semanticError(const char* message);

#endif
