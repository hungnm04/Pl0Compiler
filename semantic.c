#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"
#include "lexer.h"

static Scope* currentScope = NULL;
static int currentLevel = 0;

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


static void addSymbol(const char* name, SymbolType type) {
    if (isDeclaredInCurrentScope(name)) {
        fprintf(stderr, "Loi: Dinh danh '%s' da duoc khai bao truoc do trong cung pham vi (dong %d).\n", name, currentLine);
        exit(1);
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
    addSymbol(name, SYM_CONST);
    if (currentScope->symbols) {
        currentScope->symbols->info.value = value;
    }
}


void addVariable(const char* name) {
    addSymbol(name, SYM_VAR);
}


void addArray(const char* name, int size) {
    if (size <= 0) {
        fprintf(stderr, "Loi ngu nghia (dong %d): Kich thuoc mang '%s' phai la so nguyen duong.\n", currentLine, name);
        exit(1);
    }
    addSymbol(name, SYM_ARRAY);
    if (currentScope->symbols) {
        currentScope->symbols->info.array.size = size;
    }
}


void addProcedure(const char* name) {
    addSymbol(name, SYM_PROCEDURE);
}


void addParameter(const char* procName, const char* paramName, int isVar) {
    SymbolNode* proc = NULL;
    if (currentScope && currentScope->parent) {
        SymbolNode* s = currentScope->parent->symbols;
        while(s) {
            if (s->type == SYM_PROCEDURE && strcmp(s->name, procName) == 0) {
                proc = s;
                break;
            }
            s = s->next;
        }
    }
    if (!proc) proc = findSymbol(procName);


    if (proc == NULL || proc->type != SYM_PROCEDURE) {
        fprintf(stderr, "Loi: Khong tim thay dinh nghia thu tuc '%s' de them tham so (dong %d).\n", procName, currentLine);
        exit(1);
    }

    ParamNode* newParam = (ParamNode*)malloc(sizeof(ParamNode));
    if (!newParam) {
        fprintf(stderr, "Loi: Khong the cap phat bo nho cho tham so moi.\n");
        exit(1);
    }

    strncpy(newParam->name, paramName, MAX_IDENT_LEN);
    newParam->name[MAX_IDENT_LEN] = '\0';
    newParam->isVar = isVar;
    newParam->next = NULL;

    if (proc->info.proc.params == NULL) {
        proc->info.proc.params = newParam;
    } else {
        ParamNode* param = proc->info.proc.params;
        while (param->next != NULL) {
            param = param->next;
        }
        param->next = newParam;
    }
    proc->info.proc.paramCount++;
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
        default: return "UnknownType";
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
            printf("  Name: %-15s | Type: %-10s | Level: %d",
                   sym->name, getSymbolTypeName(sym->type), sym->level);
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
