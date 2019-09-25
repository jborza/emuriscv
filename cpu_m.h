#pragma once
#include "types.h"
#include "state.h"

void mul(State* state, word* instruction);
void mulh(State* state, word* instruction);
void mulhu(State* state, word* instruction);
void mulhsu(State* state, word* instruction);
void rem(State* state, word* instruction);
void remu(State* state, word* instruction);
void div(State* state, word* instruction);
void divu(State* state, word* instruction);
