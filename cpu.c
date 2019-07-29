#include "cpu.h"
#include "memory.h"
#include "opcodes.h"
#include "instruction.h"

#define GET_RD(instruction) (instruction >> 7) & 0x1F
#define GET_RS1(instruction) (instruction >> 15) & 0x1F
#define GET_RS2(instruction) (instruction >> 20) & 0x1F



int decode_opcode(word* instruction) {
	//risc opcodes https://klatz.co/blog/riscv-opcodes
	InstructionAny* any = instruction;
	//get lower OPCODE_BITS bits
	return any->opcode;
}

int get_rd(word* instruction) {
	return (*instruction >> 7) & 0x1f;
}

int get_rs1(word* instruction) {
	return (*instruction >> 15) & 0x1f;
}

int get_rs2(word* instruction) {
	return (*instruction >> 20) & 0x1f;
}

int set_reg(State* state, int index, word value) {
	if (index == 0) {
		//illegal instruction;
		return -1;
	}
	state->x[index] = value;
	return index;
}

inline word get_reg(State* state, int index) {
	return state->x[index];
}

void lui(State* state, word* instruction) {
	InstructionU* in = instruction;
	word value = in->data << 12;
	set_reg(state, GET_RD(*instruction), get_reg(state, GET_RD(*instruction)) | value);
}

void addi(State* state, word* instruction) {
	InstructionI* in = instruction;
	word value = get_reg(state,GET_RS1(*instruction)) + in->imm;
	set_reg(state, GET_RD(*instruction), value);
}

void slli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	word value = get_reg(state, GET_RS1(*instruction)) << in->shamt;
	set_reg(state, GET_RD(*instruction), value);
}

void srli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	word value = get_reg(state, GET_RS1(*instruction)) >> in->shamt;
	set_reg(state, GET_RD(*instruction), value);
}

void add(State* state, word* instruction) {
	InstructionR* in = instruction;
	word value = get_reg(state, GET_RS1(*instruction)) + get_reg(state, GET_RS2(*instruction));
	set_reg(state, GET_RD(*instruction), value);
}

void ecall(State* state, word* instruction) {
	ecall_callback(state);
}

void emulate_op(State* state) {
	word* instruction = fetch_next_word(state);
	if ((*instruction & MASK_LUI) == MATCH_LUI) {
		lui(state, instruction);
	}
	else if ((*instruction & MASK_ADDI) == MATCH_ADDI) {
		addi(state, instruction);
	}
	else if ((*instruction & MASK_SLLI) == MATCH_SLLI) {
		slli(state, instruction);
	}
	else if ((*instruction & MASK_SLLI) == MATCH_SLLI) {
		srli(state, instruction);
	}
	else if ((*instruction & MASK_ADD) == MATCH_ADD) {
		add(state, instruction);
	}
	else if ((*instruction & MASK_ECALL) == MATCH_ECALL) {
		ecall(state, instruction);
	}
	else {
		printf("Unknown instruction: %8X ", *instruction);
		return 1;
	}
}