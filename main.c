#include <stdio.h>
#include <stdlib.h>
#include "parser.h"
#include "lexer.h"
#include "semantic.h"
#include "codegen.h"
#include "vm.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Cach dung: %s <ten_file_dau_vao>\n", argv[0]);
        return 1;
    }
    const char *filename = argv[1];

    initLexer(filename);
    getToken();

    if (Token == NONE) {
        printf("Loi: File '%s' rong hoac token dau tien khong hop le.\n", filename);
        closeLexer();
        return 1;
    }

    Program();

    printf("Bien dich thanh cong!\n");
    printCode();
    execute();

    freeSymbolTable();
   	closeLexer();
	return 0;
}
