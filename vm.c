#include <stdio.h>
#include <stdlib.h>
#include "vm.h"
#include "codegen.h"

#define MAX_STACK 2048
int stack[MAX_STACK];

int base(int levelDiff, int currentBase) {
    int b_iter = currentBase;
    while (levelDiff > 0) {
        b_iter = stack[b_iter + 0]; 
        levelDiff--;
    }
    return b_iter;
}

void execute() {
    int pc = 0;
    int b = 0;
    int t = -1;
    Instruction i;

    printf("\n--- BAT DAU THUC THI ---\n");
    stack[0] = 0; // Static Link của global scope
    stack[1] = 0; // Dynamic Link của global scope
    stack[2] = 0; // Return Address của global scope
    t = 2;

    do {
        i = code[pc++];
        switch (i.op) {
            case OP_LA:  t++; stack[t] = base(i.p, b) + i.q; break;
            case OP_LV:  t++; stack[t] = stack[base(i.p, b) + i.q]; break;
            case OP_LC:  t++; stack[t] = i.q; break;
            case OP_LI:  stack[t] = stack[stack[t]]; break;
            case OP_INT: t += i.q; break;
            case OP_DCT: t -= i.q; break;
            case OP_J:   pc = i.q; break;
            case OP_FJ:  if (stack[t--] == 0) pc = i.q; break;
            case OP_HLT: break;
            case OP_ST:  stack[stack[t - 1]] = stack[t]; t -= 2; break;
            case OP_CALL:
                { 
                    int new_frame_base = t + 1;   
                                                
                    stack[new_frame_base + 0] = base(i.p, b); // Static Link for the new frame
                    stack[new_frame_base + 1] = b;            // Dynamic Link (caller's b)
                    stack[new_frame_base + 2] = pc;           // Return Address
                    b = new_frame_base;                       
                    t = new_frame_base + 2;                   
                    pc = i.q;                                 
                }
                break;
            case OP_EP:  t = b - 1; pc = stack[b + 2]; b = stack[b + 1]; break;
            case OP_ADD: t--; stack[t] += stack[t + 1]; break;
            case OP_SUB: t--; stack[t] -= stack[t + 1]; break;
            case OP_MUL: t--; stack[t] *= stack[t + 1]; break;
            case OP_DIV:
                t--;
                if (stack[t + 1] == 0) { printf("Loi thuc thi: Chia cho 0.\n"); return; }
                stack[t] /= stack[t + 1];
                break;
            case OP_NEG: stack[t] = -stack[t]; break;
            case OP_MOD: t--; stack[t] %= stack[t+1]; break;
            case OP_EQ:  t--; stack[t] = (stack[t] == stack[t + 1]); break;
            case OP_NE:  t--; stack[t] = (stack[t] != stack[t + 1]); break;
            case OP_GT:  t--; stack[t] = (stack[t] > stack[t + 1]); break;
            case OP_LT:  t--; stack[t] = (stack[t] < stack[t + 1]); break;
            case OP_GE:  t--; stack[t] = (stack[t] >= stack[t + 1]); break;
            case OP_LE:  t--; stack[t] = (stack[t] <= stack[t + 1]); break;
            case OP_WRI: printf("%d ", stack[t--]); break;
            case OP_WLN: printf("\n"); break;
            case OP_RI:
                printf("Nhap mot so nguyen: ");
                scanf("%d", &stack[stack[t--]]);
                break;
            default: break;
        }
    } while (i.op != OP_HLT);

    printf("\n--- KET THUC THUC THI ---\n");
}