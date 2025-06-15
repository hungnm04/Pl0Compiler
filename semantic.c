#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "lexer.h"

Scope* currentScope = NULL;
int currentLevel = 0;

void semanticError(const char* message) {
    printf("Loi ngu nghia (dong %d): %s\n", currentLine, message);
    exit(1);
}

void initSymbolTable() {
    currentScope = (Scope*)malloc(sizeof(Scope));
    if (!currentScope) {
        fprintf(stderr, "Loi: Khong the cap phat bo nho cho bang ky hieu.\n");
        exit(1);
    }
    currentScope->symbols = NULL;
    currentScope->parent = NULL;
    strcpy(currentScope->procName, "global");
    currentLevel = 0;
}

void freeSymbols(SymbolNode* head) {
    while (head != NULL) {
        SymbolNode* temp = head;
        head = head->next;

        if (temp->type == SYM_PROCEDURE) {
            ParamNode* param = temp->info.proc.params;
            while (param != NULL) {
                ParamNode* tempParam = param;
                param = param->next;
                free(tempParam);
            }
        }
        free(temp);
    }
}

void exitScope() {
    if (currentScope == NULL) return;

    Scope* parent = currentScope->parent;
    freeSymbols(currentScope->symbols);
    free(currentScope);

    currentScope = parent;
    if (currentScope != NULL) currentLevel--;
    else currentLevel = -1;
}


void freeSymbolTable() {
    while (currentScope != NULL) {
        exitScope();
    }
}


void enterScope(const char* procName) {
    Scope* newScope = (Scope*)malloc(sizeof(Scope));
    if (!newScope) {
        fprintf(stderr, "Loi: Khong the cap phat bo nho cho pham vi moi.\n");
        exit(1);
    }

    newScope->symbols = NULL;
    newScope->parent = currentScope;
    if (procName != NULL) {
        strncpy(newScope->procName, procName, MAX_IDENT_LEN);
        newScope->procName[MAX_IDENT_LEN] = '\0';
    } else {
        strcpy(newScope->procName, "anonymous_scope");
    }

    currentScope = newScope;
    currentLevel++;
}


static void addSymbol(const char* name, SymbolType type, int address) {
    if (isDeclaredInCurrentScope(name)) {
        semanticError("Dinh danh da duoc khai bao truoc do trong cung pham vi");
    }

    SymbolNode* newSymbol = (SymbolNode*)malloc(sizeof(SymbolNode));
    if (!newSymbol) {
        fprintf(stderr, "Loi: Khong the cap phat bo nho cho ky hieu moi.\n");
        exit(1);
    }

    strncpy(newSymbol->name, name, MAX_IDENT_LEN);
    newSymbol->name[MAX_IDENT_LEN] = '\0';
    newSymbol->type = type;
    newSymbol->level = currentLevel;
    newSymbol->address = address;

    if (type == SYM_PROCEDURE) {
        newSymbol->info.proc.params = NULL;
        newSymbol->info.proc.paramCount = 0;
    } else if (type == SYM_CONST) {
        newSymbol->info.value = 0;
    } else if (type == SYM_ARRAY) {
        newSymbol->info.array.size = 0;
    }

    newSymbol->next = currentScope->symbols;
    currentScope->symbols = newSymbol;
}


void addConstant(const char* name, int value) {
    addSymbol(name, SYM_CONST, 0);
    if (currentScope->symbols) {
        currentScope->symbols->info.value = value;
    }
}

void addVariable(const char* name, int level, int* address) {
    addSymbol(name, SYM_VAR, *address);
    (*address)++;
}

void addArray(const char* name, int size, int level, int* address) {
    if (size <= 0) {
        semanticError("Kich thuoc mang phai la so nguyen duong");
    }
    addSymbol(name, SYM_ARRAY, *address);
    if (currentScope->symbols) {
        currentScope->symbols->info.array.size = size;
    }
    (*address) += size;
}

SymbolNode* addProcedure(const char* name, int level, int address) {
    addSymbol(name, SYM_PROCEDURE, address);
    return currentScope->symbols;
}


void addParameter(SymbolNode* procSymbol, const char* paramName, int isVar, int level, int* address) {
    if (!procSymbol || procSymbol->type != SYM_PROCEDURE) {
        semanticError("Khong tim thay dinh nghia thu tuc de them tham so");
    }

    // Add parameter as symbol in current scope
    SymbolType paramType = isVar ? SYM_PARAM_VAR : SYM_PARAM_VAL;
    addSymbol(paramName, paramType, *address);
    (*address)++;

    // Add to procedure's parameter list
    ParamNode* newParam = (ParamNode*)malloc(sizeof(ParamNode));
    if (!newParam) {
        fprintf(stderr, "Loi: Khong the cap phat bo nho cho tham so moi.\n");
        exit(1);
    }

    strncpy(newParam->name, paramName, MAX_IDENT_LEN);
    newParam->name[MAX_IDENT_LEN] = '\0';
    newParam->isVar = isVar;
    newParam->next = NULL;

    if (procSymbol->info.proc.params == NULL) {
        procSymbol->info.proc.params = newParam;
    } else {
        ParamNode* param = procSymbol->info.proc.params;
        while (param->next != NULL) {
            param = param->next;
        }
        param->next = newParam;
    }
    procSymbol->info.proc.paramCount++;
}


SymbolNode* findSymbol(const char* name) {
    Scope* scope = currentScope;
    while (scope != NULL) {
        SymbolNode* current = scope->symbols;
        while (current != NULL) {
            if (strcmp(current->name, name) == 0) {
                return current;
            }
            current = current->next;
        }
        scope = scope->parent;
    }
    return NULL;
}


SymbolNode* findSymbolInCurrentScope(const char* name) {
    if (currentScope == NULL) return NULL;

    SymbolNode* current = currentScope->symbols;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}


int isDeclaredInCurrentScope(const char* name) {
    return findSymbolInCurrentScope(name) != NULL;
}

const char* getSymbolTypeName(SymbolType type) {
    switch (type) {
        case SYM_CONST: return "Constant";
        case SYM_VAR: return "Variable";
        case SYM_ARRAY: return "Array";
        case SYM_PROCEDURE: return "Procedure";
        case SYM_PARAM_VAL: return "Param(Val)";
        case SYM_PARAM_VAR: return "Param(Var)";
        default: return "Unknown";
    }
}

void printSymbolTable() {
    printf("\n======= SYMBOL TABLE (DEBUG) =======\n");
    Scope* s = currentScope;
    int tempLevel = currentLevel;
    while (s != NULL) {
        printf("--- Scope Level %d (Procedure: %s) ---\n", tempLevel, s->procName);
        SymbolNode* sym = s->symbols;
        if (sym == NULL) {
            printf("  (No symbols in this scope)\n");
        }
        while (sym != NULL) {
            printf("  Name: %-15s | Type: %-10s | Level: %d | Address: %d",
                   sym->name, getSymbolTypeName(sym->type), sym->level, sym->address);
            if (sym->type == SYM_CONST) {
                printf(" | Value: %d", sym->info.value);
            } else if (sym->type == SYM_ARRAY) {
                printf(" | Size: %d", sym->info.array.size);
            } else if (sym->type == SYM_PROCEDURE) {
                printf(" | ParamCount: %d", sym->info.proc.paramCount);
                ParamNode* p = sym->info.proc.params;
                if (p) printf(" | Params: ");
                while (p) {
                    printf("%s%s ", p->name, p->isVar ? "(VAR)" : "");
                    p = p->next;
                }
            }
            printf("\n");
            sym = sym->next;
        }
        s = s->parent;
        tempLevel--;
    }
    printf("====================================\n\n");
}
