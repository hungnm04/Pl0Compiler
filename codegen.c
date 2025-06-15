#include "codegen.h"
#include <stdio.h>
#include <stdlib.h>

Instruction code[MAX_CODE];
int codeIndex = 0;

void initCode() {
    codeIndex = 0;
}

void genCode(OpCode op, int p, int q) {
    if (codeIndex >= MAX_CODE) {
        printf("Loi: Bo dem ma day!\n");
        exit(1);
    }
    code[codeIndex].op = op;
    code[codeIndex].p = p;
    code[codeIndex].q = q;
    codeIndex++;
}

void patchJump(int jumpInstructionIndex, int targetAddress) {
    if (jumpInstructionIndex >= 0 && jumpInstructionIndex < codeIndex) {
        code[jumpInstructionIndex].q = targetAddress;
    }
}

const char* opCodeNames[] = {
    "LA", "LV", "LC", "LI", "INT", "DCT", "J", "FJ", "HLT", "ST",
    "CALL", "EP", "EF", "RC", "RI", "WRC", "WRI", "WLN", "ADD", "SUB",
    "MUL", "DIV", "NEG", "CV", "EQ", "NE", "GT", "LT", "GE", "LE", "MOD"
};

void printCode() {
    printf("\n--- CODE ---\n");
    for (int i = 0; i < codeIndex; i++) {
        printf("%4d: %-5s", i, opCodeNames[code[i].op]);
        switch (code[i].op) {
            case OP_LA: case OP_LV: case OP_CALL:
                printf(" %4d, %-4d", code[i].p, code[i].q);
                break;
            case OP_LC: case OP_INT: case OP_DCT: case OP_J: case OP_FJ:
                printf(" %4d", code[i].q);
                break;
            default:
                break;
        }
        printf("\n");
    }
    printf("-----------------------------\n");
}