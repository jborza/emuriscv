#pragma once
#include "state.h"
#include "cpu_ecall.h"
#include "cpu_ebreak.h"

void emulate_op(State* state);
inline word get_reg(State* state, int index);
int set_reg(State * state, int index, word value);