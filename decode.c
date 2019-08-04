#pragma once
#include "decode.h"

uint32_t bextr(uint32_t src, uint32_t start, uint32_t len) {
	return (src >> start) & ((1 << len) - 1);
}

int32_t shamt(word value) {
	return bextr(value, 20, 6);
}

uint32_t imm_sign(word value) {
	int sign = bextr(value, 31, 1);
	return sign == 1 ? (uint32_t)-1 : 0;
}

int32_t get_b_imm(word value) {
	return (bextr(value, 8, 4) << 1) + (bextr(value, 25, 6) << 5) + (bextr(value, 7, 1) << 11) + (imm_sign(value) << 12);
}