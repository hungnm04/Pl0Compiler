#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "lexer.h"
#include "semantic.h"
#include "codegen.h"


void error(const char* message) {
    printf("Loi phan tich cu phap (dong %d): %s. Token hien tai: %s\n", currentLine, message, getTokenName(Token));
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

ExpressionResult factor() {
    SymbolNode* symbol;
    ExpressionResult result = {0, 0};

    if (Token == IDENT) {
        symbol = findSymbol(Id);
        if (!symbol) semanticError("Dinh danh chua duoc khai bao");

        match(IDENT);

        if (symbol->type == SYM_CONST) {
            result.isConstant = 1;
            result.value = symbol->info.value;
            genCode(OP_LC, 0, symbol->info.value);
        } else {
            result.isConstant = 0;
            if (symbol->type == SYM_VAR || symbol->type == SYM_PARAM_VAL) {
                genCode(OP_LV, currentLevel - symbol->level, symbol->address);
            } else if (symbol->type == SYM_PARAM_VAR) {
                genCode(OP_LV, currentLevel - symbol->level, symbol->address);
                genCode(OP_LI, 0, 0);            } else if (symbol->type == SYM_ARRAY) {
                genCode(OP_LA, currentLevel - symbol->level, symbol->address);
                match(LBRACK);
                expression();
                match(RBRACK);
                genCode(OP_ADD, 0, 0);
                genCode(OP_LI, 0, 0);
            } else {
                semanticError("Dinh danh khong the su dung trong bieu thuc");
            }
        }
    } else if (Token == NUMBER) {
        result.isConstant = 1;
        result.value = Num;
        genCode(OP_LC, 0, Num);
        match(NUMBER);
    } else if (Token == LPARENT) {
        match(LPARENT);
        result = expression();
        match(RPARENT);
    } else {
        error("Loi factor: Ky vong dinh danh, so, hoac '('");
    }
    return result;
}

ExpressionResult term() {
    ExpressionResult res1 = factor();

    while (Token == TIMES || Token == SLASH || Token == PERCENT) {
        TokenType op = Token;
        match(Token);
        ExpressionResult res2 = factor();

        if (op == TIMES) genCode(OP_MUL, 0, 0);
        else if (op == SLASH) genCode(OP_DIV, 0, 0);
        else if (op == PERCENT) genCode(OP_MOD, 0, 0);
        
        res1.isConstant = 0;
    }
    return res1;
}

ExpressionResult expression() {
    int isNegative = 0;
    if (Token == PLUS || Token == MINUS) {
        if (Token == MINUS) isNegative = 1;
        match(Token);
    }
    
    ExpressionResult res1 = term();

    if (isNegative) {
        genCode(OP_NEG, 0, 0);
        res1.isConstant = 0;
    }

    while (Token == PLUS || Token == MINUS) {
        TokenType op = Token;
        match(Token);
        ExpressionResult res2 = term();

        if (op == PLUS) genCode(OP_ADD, 0, 0);
        else genCode(OP_SUB, 0, 0);
        
        res1.isConstant = 0;
    }
    return res1;
}

void condition() {
    if (Token == ODD) {
        match(ODD);
        expression();
        genCode(OP_LC, 0, 2);
        genCode(OP_MOD, 0, 0);
        genCode(OP_LC, 0, 1);
        genCode(OP_EQ, 0, 0);
    } else {
        expression();
        OpCode op;
        switch(Token) {
            case EQU: op = OP_EQ; break;
            case NEQ: op = OP_NE; break;
            case LSS: op = OP_LT; break;
            case LEQ: op = OP_LE; break;
            case GTR: op = OP_GT; break;
            case GEQ: op = OP_GE; break;
            default: error("Dieu kien loi: Ky vong toan tu quan he"); return;
        }
        match(Token);
        expression();
        genCode(op, 0, 0);
    }
}

void statement() {
    SymbolNode* symbol = NULL;
    char identName[MAX_IDENT_LEN + 1];

    if (Token == IDENT) {
        symbol = findSymbol(Id);
        if (!symbol) semanticError("Dinh danh chua duoc khai bao");
        if (symbol->type == SYM_CONST || symbol->type == SYM_PROCEDURE)
            semanticError("Khong the gan gia tri cho hang so hoac thu tuc");

        if(symbol->type == SYM_PARAM_VAR) {
             genCode(OP_LV, currentLevel - symbol->level, symbol->address);
        } else {
             genCode(OP_LA, currentLevel - symbol->level, symbol->address);
        }

        match(IDENT);
        if (Token == LBRACK) {
            if (symbol->type != SYM_ARRAY) semanticError("Dinh danh khong phai mang");
            match(LBRACK);
            expression();
            match(RBRACK);
            genCode(OP_ADD, 0, 0);
        }
        match(ASSIGN);
        expression();
        genCode(OP_ST, 0, 0);
    } else if (Token == CALL) {
        match(CALL);
        strcpy(identName, Id);
        
        if (strcmp(identName, "writei") == 0) {
            match(IDENT);
            match(LPARENT);
            expression();
            match(RPARENT);
            genCode(OP_WRI, 0, 0);
            return;
        }
        if (strcmp(identName, "writeln") == 0) {
            match(IDENT);
            genCode(OP_WLN, 0, 0);
            return;
        }
        if (strcmp(identName, "readi") == 0) {
             match(IDENT);
             match(LPARENT);
             symbol = findSymbol(Id);
             if(!symbol) semanticError("Bien de doc gia tri vao chua duoc khai bao");
             if(symbol->type != SYM_VAR && symbol->type != SYM_PARAM_VAR) semanticError("Chi co the doc gia tri vao mot bien hoac tham bien");
             
             if(symbol->type == SYM_PARAM_VAR) genCode(OP_LV, currentLevel - symbol->level, symbol->address);
             else genCode(OP_LA, currentLevel - symbol->level, symbol->address);
             
             genCode(OP_RI, 0, 0);
             match(IDENT);
             match(RPARENT);
             return;
        }

        symbol = findSymbol(identName);
        if (!symbol) semanticError("Thu tuc chua duoc khai bao");
        if (symbol->type != SYM_PROCEDURE) semanticError("Dinh danh khong phai thu tuc");
        match(IDENT);
        
        int argCount = 0;
        if(Token == LPARENT) {
            match(LPARENT);
            if (Token != RPARENT) {
                do {
                    if (argCount > 0) match(COMMA);
                    expression();
                    argCount++;
                } while (Token == COMMA);
            }
            match(RPARENT);
        }
        if (argCount != symbol->info.proc.paramCount) semanticError("So luong tham so khong khop");
        genCode(OP_CALL, currentLevel - symbol->level, symbol->address);

    } else if (Token == BEGIN) {
        match(BEGIN);
        statement();
        while (Token == SEMICOLON) {
            match(SEMICOLON);
            if (Token != END) statement();
        }
        match(END);
    } else if (Token == IF) {
        match(IF);
        condition();
        int jmpAddr1 = codeIndex;
        genCode(OP_FJ, 0, 0);
        match(THEN);
        statement();
        if (Token == ELSE) {
            match(ELSE);
            int jmpAddr2 = codeIndex;
            genCode(OP_J, 0, 0);
            patchJump(jmpAddr1, codeIndex);
            statement();
            patchJump(jmpAddr2, codeIndex);
        } else {
            patchJump(jmpAddr1, codeIndex);
        }    } else if (Token == WHILE) {
        int startLoopAddr = codeIndex;
        match(WHILE);
        condition();
        int jmpAddr = codeIndex;
        genCode(OP_FJ, 0, 0);
        match(DO);
        statement();
        genCode(OP_J, 0, startLoopAddr);
        patchJump(jmpAddr, codeIndex);    } else if (Token == FOR) {
        // FOR loop: FOR var := start TO end DO statement
        match(FOR);
        
        // Parse loop variable
        char loopVarName[MAX_IDENT_LEN + 1];
        strcpy(loopVarName, Id);
        SymbolNode* loopVar = findSymbol(Id);
        if (!loopVar) semanticError("Bien vong lap chua duoc khai bao");
        if (loopVar->type != SYM_VAR && loopVar->type != SYM_PARAM_VAR) 
            semanticError("Bien vong lap phai la bien");
        
        match(IDENT);
        match(ASSIGN);

        if(loopVar->type == SYM_PARAM_VAR) {
            genCode(OP_LV, currentLevel - loopVar->level, loopVar->address);
        } else {
            genCode(OP_LA, currentLevel - loopVar->level, loopVar->address);
        }
        

        expression();
        genCode(OP_ST, 0, 0);  
        
        match(TO);
        
        int startLoopAddr = codeIndex;
        
        // Load loop variable value for comparison
        if(loopVar->type == SYM_PARAM_VAR) {
            genCode(OP_LV, currentLevel - loopVar->level, loopVar->address);
        } else {
            genCode(OP_LA, currentLevel - loopVar->level, loopVar->address);
        }
        genCode(OP_LI, 0, 0);  
        
        expression();
        
        // Compare: loop_var <= end_value
        genCode(OP_LE, 0, 0);

        int jmpOutAddr = codeIndex;
        genCode(OP_FJ, 0, 0);
        
        match(DO);
        statement();
        
        if(loopVar->type == SYM_PARAM_VAR) {
            genCode(OP_LV, currentLevel - loopVar->level, loopVar->address);
        } else {
            genCode(OP_LA, currentLevel - loopVar->level, loopVar->address);
        }
        
        if(loopVar->type == SYM_PARAM_VAR) {
            genCode(OP_LV, currentLevel - loopVar->level, loopVar->address);
        } else {
            genCode(OP_LA, currentLevel - loopVar->level, loopVar->address);
        }
        genCode(OP_LI, 0, 0);  // Load current value
        genCode(OP_LC, 0, 1);  // Load constant 1
        genCode(OP_ADD, 0, 0); // Add 1
        genCode(OP_ST, 0, 0);  // Store back
        
        // Jump back to loop condition
        genCode(OP_J, 0, startLoopAddr);
        
        patchJump(jmpOutAddr, codeIndex);
    }
}

void block() {
    int dx = 3;  
    int jmpAddr = codeIndex;
    genCode(OP_J, 0, 0); 

    if (Token == CONST) {
        match(CONST);
        do {
            char name[MAX_IDENT_LEN+1]; strcpy(name, Id);
            match(IDENT);
            match(EQU);
            match(NUMBER);
            addConstant(name, Num);
            if (Token != COMMA) break;
            match(COMMA);
        } while (1);
        match(SEMICOLON);
    }

    if (Token == VAR) {
        match(VAR);
        do {
            char name[MAX_IDENT_LEN+1]; strcpy(name, Id);
            match(IDENT);
            if (Token == LBRACK) {
                match(LBRACK);
                match(NUMBER);
                addArray(name, Num, currentLevel, &dx);
                match(RBRACK);
            } else {
                addVariable(name, currentLevel, &dx);
            }
            if (Token != COMMA) break;
            match(COMMA);
        } while (1);
        match(SEMICOLON);
    }
    
    while (Token == PROCEDURE) {
        match(PROCEDURE);
        char procName[MAX_IDENT_LEN+1];
        strcpy(procName, Id);
        match(IDENT);
        
        SymbolNode* proc = addProcedure(procName, currentLevel, codeIndex);
        
        enterScope(procName);
        int procDx = 3;

        if (Token == LPARENT) {
            match(LPARENT);
            if (Token != RPARENT) {
                do {
                    int isVar = 0;
                    if (Token == VAR) {
                        match(VAR);
                        isVar = 1;
                    }
                    addParameter(proc, Id, isVar, currentLevel, &procDx);
                    match(IDENT);
                    if(Token != COMMA) break;
                    match(COMMA);
                } while (1);
            }
            match(RPARENT);
        }
        match(SEMICOLON);
        block();
        match(SEMICOLON);
        exitScope();
        genCode(OP_EP, 0, 0);
    }

    patchJump(jmpAddr, codeIndex);
    genCode(OP_INT, 0, dx);
    statement();
}

void Program() {
    initSymbolTable();
    initCode();
    
    match(PROGRAM);
    SymbolNode* prog = addProcedure(Id, -1, 0);
    match(IDENT);
    match(SEMICOLON);
    
    enterScope(prog->name);
    block();
    exitScope();

    match(PERIOD);
    genCode(OP_HLT, 0, 0);
}