#include "cpu.h"
#include "memory.h"

int decode_opcode(word instruction) {
	//risc opcodes https://klatz.co/blog/riscv-opcodes
	switch (instruction) {
		break;
		
	}
	return -1;
}

void emulate_op(State* state) {
	word* instruction = fetch_next_word(state);
	//assume U-type
	int opcode = decode_opcode(instruction);
}