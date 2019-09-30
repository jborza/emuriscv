#pragma once
#include "config.h"
#include "types.h"
#include "state.h"

/* "A" Standard Extension for AtomicInstructions, Version 2.0 */

void amoadd_w(State* state, word* instruction);
void amoand_w(State* state, word* instruction);
void amoor_w(State* state, word* instruction);
void amoxor_w(State* state, word* instruction); 
void amoswap_w(State* state, word* instruction);
void lr(State* state, word* instruction);
void sc(State* state, word* instruction);