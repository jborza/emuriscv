#include "cpu.h"
#include "cpu_m.h"
#include "debug.h"

static inline int32_t mul32(int32_t a, int32_t b)
{
	return (int32_t)(a * b);
}

static inline uint32_t mulh32(int32_t a, int32_t b)
{
	return ((int64_t)a * (int64_t)b) >> 32;
}

static inline uint32_t mulhsu32(int32_t a, uint32_t b)
{
	return ((int64_t)a * (int64_t)b) >> 32;
}

static inline uint32_t mulhu32(uint32_t a, uint32_t b)
{
	return ((int64_t)a * (int64_t)b) >> 32;
}

static inline int32_t div32(int32_t a, int32_t b)
{
    if (b == 0) {
        return -1;
    } else if (a == ((int32_t)1 << (XLEN - 1)) && b == -1) {
        return a;
    } else {
        return a / b;
    }
}

static inline uint32_t divu32(uint32_t a, uint32_t b)
{
    if (b == 0) {
        return -1;
    } else {
        return a / b;
    }
}

static inline int32_t rem32(int32_t a, int32_t b)
{
    if (b == 0) {
        return a;
    } else if (a == ((int32_t)1 << (XLEN - 1)) && b == -1) {
        return 0;
    } else {
        return a % b;
    }
}

static inline uint32_t remu32(uint32_t a, uint32_t b)
{
    if (b == 0) {
        return a;
    } else {
        return a % b;
    }
}

#define M_OP(NAME, OP) { \
	PRINT_DEBUG(NAME" x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction)); \
	uint32_t a = get_rs1_value(state, instruction); \
	uint32_t b = get_rs2_value(state, instruction); \
	set_rd_value(state, instruction, OP(a, b)); \
}

void mul(State* state, word* instruction) {
	PRINT_DEBUG("mul x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	uint32_t a = get_rs1_value(state, instruction);
	uint32_t b = get_rs2_value(state, instruction);
	int32_t result = mul32(a, b);
	set_rd_value(state, instruction, result);
}

void mulh(State* state, word* instruction) {
	M_OP("mulh", mulh32);
}
void mulhu(State* state, word* instruction) {
	M_OP("mulhu", mulhu32);
}

void mulhsu(State* state, word* instruction) {
	M_OP("mulhsu", mulhsu32);
}
void rem(State* state, word* instruction) {
	M_OP("rem", rem32);
}
void remu(State* state, word* instruction) {
	M_OP("remu", remu32);
}
void div(State* state, word* instruction) {
	M_OP("div", div32);
}
void divu(State* state, word* instruction) {
	M_OP("divu", divu32);
}
