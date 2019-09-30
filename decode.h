#pragma once
#include "types.h"

uint32_t bextr(uint32_t src, uint32_t start, uint32_t len);

int32_t shamt(word value);

uint32_t imm_sign(word value);

int32_t get_b_imm(word value);

int32_t get_i_imm(word value);
uint32_t get_i_imm_unsigned(word value);

int32_t get_j_imm(word value);

int32_t get_s_imm(word value);