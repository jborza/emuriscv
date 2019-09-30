#pragma once
#include "state.h"

void emulate_op(State* state);
word get_reg(State* state, int index);
int set_reg(State * state, int index, word value);
word get_rs1_value(State* state, word* instruction);
word get_rs2_value(State* state, word* instruction);
int set_rd_value(State* state, word* instruction, word value);

#define GET_RD(x) ((x >> 7) & 0x1F)
#define GET_RS1(x) ((x >> 15) & 0x1F)
#define GET_CSR_IMM(x) ((x >> 15) & 0x1F)
#define GET_RS2(x) ((x >> 20) & 0x1F)