#include "lexer.h"

TokenType Token;
int Num;
char Id[MAX_IDENT_LEN+1];
char Ch;
FILE* input;
int currentLine = 1; 

char getCh(){
	Ch = fgetc(input);
	if (Ch == '\n') {
	    currentLine++; 
	}
	return Ch;
}     

const char* getTokenName(TokenType token) {
    static const char* tokenNames[] = {
        "NONE", "IDENT", "NUMBER",
        "BEGIN", "CALL", "CONST", "DO", "ELSE", "END", "FOR", "IF", "ODD",
        "PROCEDURE", "PROGRAM", "THEN", "TO", "VAR", "WHILE",
        "PLUS", "MINUS", "TIMES", "SLASH", "EQU", "NEQ", "LSS", "LEQ",
        "GTR", "GEQ", "LPARENT", "RPARENT", "LBRACK", "RBRACK",
        "PERIOD", "COMMA", "SEMICOLON", "ASSIGN", "PERCENT"
    };
    return tokenNames[token];
}

TokenType check(char* str) {
    if (strcmp(str, "begin") == 0) return BEGIN;
    if (strcmp(str, "call") == 0) return CALL;
    if (strcmp(str, "const") == 0) return CONST;
    if (strcmp(str, "do") == 0) return DO;
    if (strcmp(str, "else") == 0) return ELSE;
    if (strcmp(str, "end") == 0) return END;
    if (strcmp(str, "for") == 0) return FOR;
    if (strcmp(str, "if") == 0) return IF;
    if (strcmp(str, "odd") == 0) return ODD;
    if (strcmp(str, "procedure") == 0) return PROCEDURE;
    if (strcmp(str, "program") == 0) return PROGRAM;
    if (strcmp(str, "then") == 0) return THEN;
    if (strcmp(str, "to") == 0) return TO;
    if (strcmp(str, "var") == 0) return VAR;
    if (strcmp(str, "while") == 0) return WHILE;
    
    return IDENT;  
}

TokenType getToken() {
    // Skip whitespace
    while (isspace(Ch)) getCh();
    
    if (isalpha(Ch)) { 
        int i = 0;
        do {
            if (i < MAX_IDENT_LEN) Id[i++] = tolower(Ch);
            getCh();
        } while (isalnum(Ch));
        Id[i] = '\0';
        return (Token = check(Id));
    }
    else if (isdigit(Ch)) { // Number
        Num = 0;
        do {
            Num = Num * 10 + (Ch - '0');
            getCh();
        } while (isdigit(Ch));
        return (Token = NUMBER);
    }
    else switch (Ch) { // Operators and delimiters
        case '+': 
            getCh(); 
            return (Token = PLUS);
        case '-': 
            getCh(); 
            return (Token = MINUS);
        case '*': 
            getCh(); 
            return (Token = TIMES);
        case '/': 
            getCh(); 
            return (Token = SLASH);
        case '=': 
            getCh(); 
            return (Token = EQU);
        case '<': 
            getCh();
            if (Ch == '=') { 
                getCh(); 
                return (Token = LEQ); 
            }
            else if (Ch == '>') { 
                getCh(); 
                return (Token = NEQ); 
            }
            else return (Token = LSS);
        case '>':
            getCh();
            if (Ch == '=') {
                getCh();
                return (Token = GEQ);
            }
            else return (Token = GTR);
        case '(':
            getCh();
            return (Token = LPARENT);
        case ')':
            getCh();
            return (Token = RPARENT);
        case '[':
            getCh();
            return (Token = LBRACK);
        case ']':
            getCh();
            return (Token = RBRACK);
        case '.':
            getCh();
            return (Token = PERIOD);
        case ',':
            getCh();
            return (Token = COMMA);
        case ';':
            getCh();
            return (Token = SEMICOLON);
        case ':':
            getCh();
            if (Ch == '=') {
                getCh();
                return (Token = ASSIGN);
            }
            printf("Error: Expected '=' after ':'\n");
            break;
        case '%':
            getCh();
            return (Token = PERCENT);
        case EOF:
            return (Token = NONE);
        default:
            printf("Unknown character: %c (ASCII: %d)\n", Ch, Ch);
            getCh();
            return getToken();
    }
    return (Token = NONE);
}

void printToken() {
    printf("%s", getTokenName(Token));
    if (Token == IDENT) {
        printf("(%s)", Id);
    } else if (Token == NUMBER) {
        printf("(%d)", Num);
    }
    printf("\n");
}

void initLexer(const char* File){
	input = fopen(File, "r");
	if(!input){
		exit(1);
	}
	Ch = ' ';
}

void closeLexer() {
    if (input) fclose(input);
}



