#include "memory.h"

word* fetch_next_word(State* state) {
	word* address = state->memory + state->pc;
	state->pc += 4;
	return address;
}

void write_word(State* state, word address, word value){
	state->memory[address] = value & 0xff;
	state->memory[address + 1] = (value & 0xff00) >> 8;
	state->memory[address + 2] = (value & 0xff0000) >> 16;
	state->memory[address + 3] = (value & 0xff000000) >> 24;
}

word read_word(State* state, word address) {
	word value = state->memory[address] |
		state->memory[address + 1] << 8 |
		state->memory[address + 2] << 16 |
		state->memory[address + 3] << 24;
}