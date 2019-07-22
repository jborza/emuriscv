#include "cpu.h"
#include "memory.h"
#include "opcodes.h"
#include "instruction.h"

int decode_opcode(word *instruction) {
	//risc opcodes https://klatz.co/blog/riscv-opcodes
	InstructionAny *any = instruction;
	//get lower OPCODE_BITS bits
	return any->opcode;
}

int set_reg(State* state, int index, word value) {
	if (index == 0) {
		//illegal instruction;
		return -1;
	}
	state->x[index] = value;
	return index;
}

void emulate_op(State* state) {
	word *instruction = fetch_next_word(state);
	//assume U-type
	int opcode = decode_opcode(instruction);
	switch (opcode)
	{
	case LUI:
		InstructionU *in = instruction;
		set_reg(state, in->rd, in->data);
		break;
	case OP_IMM:
	default:
		break;
	}
}