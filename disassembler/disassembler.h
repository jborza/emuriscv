#include <stdio.h>
#include "../types.h"
void print_debug(const char* s, ...);
#define PRINT_DEBUG(...) print_debug(__VA_ARGS__)

#define GET_RD(x) ((x >> 7) & 0x1F)
#define GET_RS1(x) ((x >> 15) & 0x1F)
#define GET_CSR_IMM(x) ((x >> 15) & 0x1F)
#define GET_RS2(x) ((x >> 20) & 0x1F)
void disassemble_op(word *instruction, word offset);
