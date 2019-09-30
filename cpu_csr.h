#pragma once
#include "types.h"
#include "state.h"

/* "Zicsr" Control and Status Register Instructions */
void csrrw(State* state, word* instruction);
void csrrs(State* state, word* instruction);
void csrrc(State* state, word* instruction);
void csrrwi(State* state, word* instruction);
void csrrsi(State* state, word* instruction);
void csrrci(State* state, word* instruction);