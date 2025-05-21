#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "semantic.h"

void semanticError(const char* message) {
    fprintf(stderr, "Loi ngu nghia (dong %d): %s. Token hien tai: %s",
            currentLine,
            message, getTokenName(Token));
    if (Token == IDENT) fprintf(stderr, " (%s)", Id);
    if (Token == NUMBER) fprintf(stderr, " (%d)", Num);
    fprintf(stderr, "\n");
    exit(1);
}

void error(const char* message) {
    fprintf(stderr, "Loi phan tich cu phap (dong %d): %s. Token hien tai: %s",
            currentLine,
            message, getTokenName(Token));
    if (Token == IDENT) fprintf(stderr, " (%s)", Id);
    if (Token == NUMBER) fprintf(stderr, " (%d)", Num);
    fprintf(stderr, "\n");
    exit(1);
}

void match(TokenType expected) {
    if (Token == expected) {
        getToken();
    } else {
        char error_msg[150];
        snprintf(error_msg, sizeof(error_msg), "Ky vong token '%s', nhung nhan duoc '%s'",
                 getTokenName(expected), getTokenName(Token));
        error(error_msg);
    }
}

SymbolType expression();
SymbolType term();
SymbolType factor();
SymbolType condition();
void statement();
void block(const char* currentProcedureName);


SymbolType factor() {
    SymbolNode* symbol;
    char currentIdFactor[MAX_IDENT_LEN + 1];
    SymbolType factorTypeResult = SYM_VAR;

    if (Token == IDENT) {
        strcpy(currentIdFactor, Id);
        symbol = findSymbol(currentIdFactor);
        if (!symbol) {
            char msg[100];
            snprintf(msg, sizeof(msg), "Dinh danh '%s' chua duoc khai bao", currentIdFactor);
            semanticError(msg);
        }

        getToken();

        if (symbol) {
            factorTypeResult = symbol->type;
            if (Token == LBRACK) {
                if (symbol->type != SYM_ARRAY) {
                    char msg[120];
                    snprintf(msg, sizeof(msg), "Dinh danh '%s' khong phai la mot mang khi truy cap phan tu", currentIdFactor);
                    semanticError(msg);
                }
                match(LBRACK);
                SymbolType indexType = expression();
                if (indexType != SYM_VAR && indexType != SYM_CONST) {
                    semanticError("Chi so mang phai la kieu so nguyen (var hoac const).");
                }
                match(RBRACK);
                factorTypeResult = SYM_VAR;
            }
        } else {
            if (Token == LBRACK) {
                 match(LBRACK);
                 expression();
                 match(RBRACK);
            }
            return SYM_VAR;
        }

    } else if (Token == NUMBER) {
        getToken();
        factorTypeResult = SYM_CONST;
    } else if (Token == LPARENT) {
        getToken();
        factorTypeResult = expression();
        match(RPARENT);
    } else {
        error("Loi factor: Ky vong dinh danh, so, hoac '('");
    }
    return factorTypeResult;
}

SymbolType term() {
    SymbolType type1, type2;
    type1 = factor();

    if (type1 == SYM_PROCEDURE) {
        semanticError("Toan hang khong hop le: khong the dung thu tuc trong phep tinh.");
    } else if (type1 == SYM_ARRAY && (Token == TIMES || Token == SLASH || Token == PERCENT)) {
        semanticError("Toan hang khong hop le: ten mang khong the dung truc tiep voi toan tu *, /, %.");
    }

    while (Token == TIMES || Token == SLASH || Token == PERCENT) {
        if (type1 == SYM_ARRAY) {
             semanticError("Toan hang khong hop le: ten mang khong the dung truc tiep trong phep tinh.");
        }
        getToken();
        type2 = factor();
        if (type2 == SYM_PROCEDURE || type2 == SYM_ARRAY) {
            semanticError("Toan hang khong hop le: khong the dung thu tuc hoac ten mang trong phep tinh.");
        }
        type1 = SYM_VAR;
    }
    return type1;
}

SymbolType expression() {
    SymbolType type1, type2;
    int unaryOp = 0;
    if (Token == PLUS || Token == MINUS) {
        unaryOp = 1;
        getToken();
    }
    type1 = term();

    if (type1 == SYM_PROCEDURE) {
         semanticError("Toan hang khong hop le: khong the dung thu tuc trong bieu thuc so hoc.");
    } else if (type1 == SYM_ARRAY && (unaryOp || Token == PLUS || Token == MINUS)) {
        semanticError("Toan hang khong hop le: ten mang khong the dung truc tiep trong bieu thuc so hoc.");
    }

    while (Token == PLUS || Token == MINUS) {
        if (type1 == SYM_ARRAY) {
             semanticError("Toan hang khong hop le: ten mang khong the dung truc tiep trong bieu thuc so hoc.");
        }
        getToken();
        type2 = term();
        if (type2 == SYM_PROCEDURE || type2 == SYM_ARRAY) {
            semanticError("Toan hang khong hop le: khong the dung thu tuc hoac ten mang trong bieu thuc so hoc.");
        }
        type1 = SYM_VAR;
    }
    return type1;
}

SymbolType condition() {
    SymbolType exprTypeLeft, exprTypeRight;
    if (Token == ODD) {
        getToken();
        exprTypeLeft = expression();
        if (exprTypeLeft == SYM_PROCEDURE || exprTypeLeft == SYM_ARRAY) {
            semanticError("ODD chi ap dung cho bieu thuc so nguyen (khong phai thu tuc/mang).");
        }
    } else {
        exprTypeLeft = expression();
        if (exprTypeLeft == SYM_PROCEDURE || exprTypeLeft == SYM_ARRAY) {
            semanticError("Toan hang so sanh khong hop le: khong the dung thu tuc hoac ten mang.");
        }

        if (Token == EQU || Token == NEQ || Token == LSS ||
            Token == LEQ || Token == GTR || Token == GEQ) {
            getToken();
            exprTypeRight = expression();
            if (exprTypeRight == SYM_PROCEDURE || exprTypeRight == SYM_ARRAY) {
                semanticError("Toan hang so sanh khong hop le: khong the dung thu tuc hoac ten mang.");
            }
        } else {
            error("Dieu kien loi: Ky vong toan tu quan he (=, <>, <, <=, >, >=)");
        }
    }
    return SYM_VAR;
}

void statement() {
    SymbolNode* symbolLHS = NULL;
    char idName[MAX_IDENT_LEN + 1];
    SymbolType typeRHS;

    switch (Token) {
        case IDENT: {
            strcpy(idName, Id);
            symbolLHS = findSymbol(idName);

            if (!symbolLHS) {
                char msg[100];
                snprintf(msg, sizeof(msg), "Dinh danh '%s' chua duoc khai bao", idName);
                semanticError(msg);
            } else {
                if (symbolLHS->type == SYM_CONST) {
                    char msg[100];
                    snprintf(msg, sizeof(msg), "Khong the gan gia tri cho hang so '%s'", idName);
                    semanticError(msg);
                }
                if (symbolLHS->type == SYM_PROCEDURE) {
                    char msg[100];
                    snprintf(msg, sizeof(msg), "Khong the gan gia tri cho thu tuc '%s'", idName);
                    semanticError(msg);
                }
            }

            getToken();

            if (Token == LBRACK) {
                if (!symbolLHS || symbolLHS->type != SYM_ARRAY) {
                    char msg[120];
                    snprintf(msg, sizeof(msg), "Dinh danh '%s' khong phai la mot mang de truy cap phan tu", idName);
                    semanticError(msg);
                }
                match(LBRACK);
                SymbolType indexType = expression();
                if (indexType != SYM_VAR && indexType != SYM_CONST) {
                     semanticError("Chi so mang phai la kieu so nguyen.");
                }
                match(RBRACK);
            } else {
                 if(symbolLHS && symbolLHS->type != SYM_VAR) {
                    char msg[120];
                    snprintf(msg, sizeof(msg), "'%s' phai la mot bien (khong phai mang/hang/thu tuc) de co the gan gia tri", idName);
                    semanticError(msg);
                 }
            }
            match(ASSIGN);
            typeRHS = expression();
            if (typeRHS == SYM_PROCEDURE || typeRHS == SYM_ARRAY) {
                semanticError("Ve phai cua phep gan khong the la thu tuc hoac ten mang (can gia tri).");
            }
            break;
        }
        case CALL: {
            getToken();
            strcpy(idName, Id);
            match(IDENT);

            SymbolNode* procSymbol = findSymbol(idName);
            if (!procSymbol) {
                char msg[100];
                snprintf(msg, sizeof(msg), "Thu tuc '%s' chua duoc khai bao", idName);
                semanticError(msg);
            } else if (procSymbol->type != SYM_PROCEDURE) {
                char msg[100];
                snprintf(msg, sizeof(msg), "'%s' khong phai la mot thu tuc", idName);
                semanticError(msg);
            }

            int actualArgCount = 0;
            ParamNode* formalParam = procSymbol ? procSymbol->info.proc.params : NULL;

            if (Token == LPARENT) {
                getToken();
                if (Token != RPARENT) {
                    if (formalParam) {
                        if (formalParam->isVar) {
                            if (Token != IDENT) {
                                semanticError("Doi so cho tham so VAR phai la mot dinh danh (bien/mang).");
                            }
                            char argName[MAX_IDENT_LEN + 1];
                            strcpy(argName, Id);
                            SymbolNode* argSymbol = findSymbol(argName);
                            if (!argSymbol) {
                                char msg[120];
                                snprintf(msg, sizeof(msg), "Dinh danh doi so '%s' cho tham so VAR chua khai bao", argName);
                                semanticError(msg);
                            }
                            getToken();
                            if (argSymbol) {
                                if (argSymbol->type == SYM_ARRAY) {
                                    if (Token != LBRACK) {
                                        semanticError("Doi so mang cho tham so VAR phai duoc truy cap phan tu.");
                                    }
                                    match(LBRACK);
                                    SymbolType indexType = expression();
                                    if (indexType != SYM_VAR && indexType != SYM_CONST) {
                                        semanticError("Chi so mang (trong doi so VAR) phai la so nguyen.");
                                    }
                                    match(RBRACK);
                                } else if (argSymbol->type != SYM_VAR) {
                                    semanticError("Doi so cho tham so VAR phai la mot bien hoac phan tu mang.");
                                }
                            }
                        } else {
                            typeRHS = expression();
                            if (typeRHS == SYM_PROCEDURE || typeRHS == SYM_ARRAY) {
                                 semanticError("Doi so gia tri khong the la ten thu tuc hoac ten mang.");
                            }
                        }
                        formalParam = formalParam->next;
                    } else {
                        expression();
                    }
                    actualArgCount++;

                    while (Token == COMMA) {
                        match(COMMA);
                         if (formalParam) {
                            if (formalParam->isVar) {
                                if (Token != IDENT) {
                                    semanticError("Doi so cho tham so VAR phai la mot dinh danh (bien/mang).");
                                }
                                char argName[MAX_IDENT_LEN + 1];
                                strcpy(argName, Id);
                                SymbolNode* argSymbol = findSymbol(argName);
                                if (!argSymbol) {
                                    char msg[120];
                                    snprintf(msg, sizeof(msg), "Dinh danh doi so '%s' cho tham so VAR chua khai bao", argName);
                                    semanticError(msg);
                                }
                                getToken();
                                if (argSymbol) {
                                    if (argSymbol->type == SYM_ARRAY) {
                                        if (Token != LBRACK) {
                                            semanticError("Doi so mang cho tham so VAR phai duoc truy cap phan tu.");
                                        }
                                        match(LBRACK);
                                        SymbolType indexType = expression();
                                        if (indexType != SYM_VAR && indexType != SYM_CONST) {
                                            semanticError("Chi so mang (trong doi so VAR) phai la so nguyen.");
                                        }
                                        match(RBRACK);
                                    } else if (argSymbol->type != SYM_VAR) {
                                        semanticError("Doi so cho tham so VAR phai la mot bien hoac phan tu mang.");
                                    }
                                }
                            } else {
                                typeRHS = expression();
                                if (typeRHS == SYM_PROCEDURE || typeRHS == SYM_ARRAY) {
                                    semanticError("Doi so gia tri khong the la ten thu tuc hoac ten mang.");
                                }
                            }
                            formalParam = formalParam->next;
                        } else {
                             expression();
                        }
                        actualArgCount++;
                    }
                }
                match(RPARENT);
            }

            if (procSymbol && procSymbol->type == SYM_PROCEDURE) {
                if (procSymbol->info.proc.paramCount != actualArgCount) {
                    char msg[200];
                    snprintf(msg, sizeof(msg), "Loi: So luong tham so khong dung. Thu tuc '%s' can %d, nhan duoc %d (dong %d)",
                             idName, procSymbol->info.proc.paramCount, actualArgCount, currentLine);
                    semanticError(msg);
                }
            }
            break;
        }
        case BEGIN:
            getToken();
            statement();
            while (Token == SEMICOLON) {
                match(SEMICOLON);
                if (Token != END) {
                    statement();
                }
            }
            match(END);
            break;
        case IF:
            getToken();
            condition();
            match(THEN);
            statement();
            if (Token == ELSE) {
                match(ELSE);
                statement();
            }
            break;
        case WHILE:
            getToken();
            condition();
            match(DO);
            statement();
            break;
        case FOR: {
            getToken();
            strcpy(idName, Id);
            match(IDENT);

            symbolLHS = findSymbol(idName);
            if (!symbolLHS) {
                char msg[100];
                snprintf(msg, sizeof(msg), "Bien lap '%s' chua duoc khai bao", idName);
                semanticError(msg);
            } else if (symbolLHS->type != SYM_VAR) {
                char msg[100];
                snprintf(msg, sizeof(msg), "Bien lap '%s' phai la mot bien", idName);
                semanticError(msg);
            }

            match(ASSIGN);
            typeRHS = expression();
            if (typeRHS == SYM_PROCEDURE || typeRHS == SYM_ARRAY) {
                semanticError("Gia tri khoi tao cho FOR khong hop le (khong phai thu tuc/mang).");
            }
            match(TO);
            typeRHS = expression();
             if (typeRHS == SYM_PROCEDURE || typeRHS == SYM_ARRAY) {
                semanticError("Gia tri ket thuc cho FOR khong hop le (khong phai thu tuc/mang).");
            }
            match(DO);
            statement();
            break;
        }
        case SEMICOLON:
        case END:
        case ELSE:
        case PERIOD:
            break;
        default:
            if (Token != NONE) {
                 error("Bat dau cau lenh khong hop le");
            }
    }
}

void block(const char* currentProcedureName) {
    char identName[MAX_IDENT_LEN + 1];

    if (Token == CONST) {
        getToken();
        do {
            strcpy(identName, Id);
            match(IDENT);
            match(EQU);
            match(NUMBER);
            addConstant(identName, Num);
            if (Token == COMMA) {
                match(COMMA);
            } else {
                break;
            }
        } while (Token == IDENT);
        match(SEMICOLON);
    }

    if (Token == VAR) {
        getToken();
        do {
            strcpy(identName, Id);
            match(IDENT);
            if (Token == LBRACK) {
                match(LBRACK);
                match(NUMBER);
                addArray(identName, Num);
                match(RBRACK);
            } else {
                addVariable(identName);
            }

            if (Token == COMMA) {
                match(COMMA);
            } else {
                break;
            }
        } while (Token == IDENT);
        match(SEMICOLON);
    }

    while (Token == PROCEDURE) {
        getToken();
        strcpy(identName, Id);
        match(IDENT);

        addProcedure(identName);
        enterScope(identName);

        if (Token == LPARENT) {
            match(LPARENT);
            if (Token != RPARENT) {
                int isVarParam = 0;
                if (Token == VAR) {
                    match(VAR);
                    isVarParam = 1;
                }
                char paramName[MAX_IDENT_LEN + 1];
                strcpy(paramName, Id);
                match(IDENT);
                addVariable(paramName);
                addParameter(identName, paramName, isVarParam);

                while (Token == COMMA) {
                    match(COMMA);
                    isVarParam = 0;
                    if (Token == VAR) {
                        match(VAR);
                        isVarParam = 1;
                    }
                    strcpy(paramName, Id);
                    match(IDENT);
                    addVariable(paramName);
                    addParameter(identName, paramName, isVarParam);
                }
            }
            match(RPARENT);
        }
        match(SEMICOLON);

        block(identName);

        match(SEMICOLON);
        exitScope();
    }
    statement();
}

void Program() {
    initSymbolTable();

    match(PROGRAM);
    match(IDENT);
    match(SEMICOLON);

    block(NULL);

    match(PERIOD);

    if (Token != NONE) {
        char error_msg[200];
        snprintf(error_msg, sizeof(error_msg),
                 "Ky tu khong mong doi '%s' sau khi ket thuc chuong trinh (sau dau '.')",
                 getTokenName(Token));
        if (Token == IDENT) {
             char specific_error_msg[250];
             snprintf(specific_error_msg, sizeof(specific_error_msg), "%s (%s)", error_msg, Id);
             error(specific_error_msg);
        } else {
            error(error_msg);
        }
    }

    freeSymbolTable();
    printf("Phan tich cu phap va ngu nghia hoan tat.\n");
}
