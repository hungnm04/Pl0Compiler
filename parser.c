#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"

void error(const char* message) {
    fprintf(stderr, "Loi phan tich cu phap: %s. Token hien tai: %s", message, getTokenName(Token));
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

void factor() {
    if (Token == IDENT) {
        getToken();
    } else if (Token == NUMBER) {
        getToken();
    } else if (Token == LPARENT) {
        getToken();
        expression();
        match(RPARENT);
    } else {
        error("Loi factor: Ky vong dinh danh, so, hoac '('");
    }
}

void term() {
    factor();
    while (Token == TIMES || Token == SLASH) {
        getToken();
        factor();
    }
}

void expression() {
    if (Token == PLUS || Token == MINUS) {
        getToken();
    }
    term();
    while (Token == PLUS || Token == MINUS) {
        getToken();
        term();
    }
}

void condition() {
    if (Token == ODD) {
        getToken();
        expression();
    } else {
        expression();
        if (Token == EQU || Token == NEQ || Token == LSS ||
            Token == LEQ || Token == GTR || Token == GEQ) {
            getToken();
            expression();
        } else {
            error("Dieu kien loi: Ky vong toan tu quan he (=, <>, <, <=, >, >=) hoac ODD"); // Updated error msg
        }
    }
}

void statement() {
    switch (Token) {
        case IDENT:
            getToken();
            match(ASSIGN);
            expression();
            break;

        case CALL:
            getToken();
            match(IDENT);
            if (Token == LPARENT) {
                getToken();
                if (Token != RPARENT) {
                    expression();
                    while (Token == COMMA) {
                        getToken();
                        expression();
                    }
                }
                match(RPARENT);
            }
            break;

        case BEGIN:
            getToken();
            statement();
            while (Token == SEMICOLON) {
                getToken();
                statement();
            }
            match(END);
            break;

        case IF:
            getToken();
            condition();
            match(THEN);
            statement();
            if (Token == ELSE) {
                 getToken();
                 statement();
            }
            break;

        case WHILE:
            getToken();
            condition();
            match(DO);
            statement();
            break;

        case FOR:
             getToken();
             match(IDENT);
             match(ASSIGN);
             expression();
             match(TO);
             expression();
             match(DO);
             statement();
             break;

        case SEMICOLON:
        case END:
        case ELSE:
        case PERIOD:
             break;

        default:
             error("Bat dau cau lenh khong hop le");
    }
}

void block() {
    if (Token == CONST) {
        getToken(); 
        match(IDENT);   
        match(EQU);     
        match(NUMBER);  
        while (Token == COMMA) { 
            getToken();  
            match(IDENT); 
            match(EQU);  
            match(NUMBER);
        }
        match(SEMICOLON); 
    }

    if (Token == VAR) {
        getToken(); 
        match(IDENT); 
        while (Token == COMMA) { 
            getToken();  
            match(IDENT); 
        }
        match(SEMICOLON); 
    }

    while (Token == PROCEDURE) {
         getToken();      
         match(IDENT);    
         match(SEMICOLON);
         block();         
         match(SEMICOLON);
     }

    statement();
}

void Program() {
    match(PROGRAM);
    match(IDENT);
    match(SEMICOLON);
    block();
    match(PERIOD);
}
