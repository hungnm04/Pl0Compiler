#ifndef CODEGEN_H
#define CODEGEN_H

typedef enum {
    OP_LA, OP_LV, OP_LC, OP_LI, OP_INT, OP_DCT, OP_J, OP_FJ, OP_HLT,
    OP_ST, OP_CALL, OP_EP, OP_EF, OP_RC, OP_RI, OP_WRC, OP_WRI, OP_WLN,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEG, OP_CV, OP_EQ, OP_NE, OP_GT,
    OP_LT, OP_GE, OP_LE, OP_MOD
} OpCode;

typedef struct {
    OpCode op;
    int p;
    int q;
} Instruction;

#define MAX_CODE 2048
extern Instruction code[MAX_CODE];
extern int codeIndex;

void initCode();
void genCode(OpCode op, int p, int q);
void patchJump(int jumpInstructionIndex, int targetAddress);
void printCode();

#endif