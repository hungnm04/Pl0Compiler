#ifndef SEMANTIC_H
#define SEMANTIC_H
#include "lexer.h"

typedef enum {
    SYM_CONST,     
    SYM_VAR,       
    SYM_ARRAY,     
    SYM_PROCEDURE 
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
    union {
        int value;              
        struct {
            int size;           
        } array;
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
} Scope;

void initSymbolTable();

void freeSymbolTable();


void enterScope(const char* procName);


void exitScope();


void addConstant(const char* name, int value);


void addVariable(const char* name);


void addArray(const char* name, int size);


void addProcedure(const char* name);


void addParameter(const char* procName, const char* paramName, int isVar);


SymbolNode* findSymbol(const char* name);


SymbolNode* findSymbolInCurrentScope(const char* name);

int isDeclaredInCurrentScope(const char* name);


int getProcedureParamCount(const char* name);

int checkProcedureCall(const char* name, int argCount);


const char* getSymbolTypeName(SymbolType type);


void printSymbolTable();

#endif
