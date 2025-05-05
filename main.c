#include <stdio.h>  
#include <stdlib.h> 
#include "parser.h" 
#include "lexer.h" 

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <input_filename>\n", argv[0]);
        return 1; 
    }
    const char *filename = argv[1];

    initLexer(filename);
    getToken(); 

    if (Token == NONE) {
        fprintf(stderr, "Loi: File '%s' rong hoac token dau tien khong hop le.\n", filename);
        closeLexer();
        return 1;    
    }

    Program();

   closeLexer();
	fprintf(stderr, "Phan tich cu phap thanh cong!\n"); 
	return 0; 
}
