#include "memory.h"

word* fetch_next_word(State* state) {
	word* address = state->memory + state->pc;
	state->pc += 4;
	return address;
}

//TODO is this little-endian?
void write_word(State* state, word address, word value) {
	state->memory[address] = value & 0xff;
	state->memory[address + 1] = (value & 0xff00) >> 8;
	state->memory[address + 2] = (value & 0xff0000) >> 16;
	state->memory[address + 3] = (value & 0xff000000) >> 24;
}

void write_halfword(State* state, word address, halfword value) {
	state->memory[address] = value & 0xff;
	state->memory[address + 1] = (value & 0xff00) >> 8;
}

void write_byte(State* state, word address, byte value) {
	state->memory[address] = value & 0xff;
}

//TODO is this little-endian?
word read_word(State* state, word address) {
	word value = state->memory[address] |
		state->memory[address + 1] << 8 |
		state->memory[address + 2] << 16 |
		state->memory[address + 3] << 24;
}

word read_halfword_signed(State* state, word address) {
	word value = state->memory[address] |
		state->memory[address + 1] << 8;
	//sign extend 16-bit value
	if ((value & 0x8000) == 0x8000)
		return value | 0xffff0000;
	else
		return value;
}

word read_halfword_unsigned(State * state, word address) {
	word value = state->memory[address] |
		state->memory[address + 1] << 8;
	return value;
}


word read_byte_signed(State* state, word address) {
	word value = state->memory[address];
	//sign extend 8-bit value
	if ((value & 0x80) == 0x80)
		return value | 0xffffff00;
	else
		return value;
}

word read_byte_unsigned(State* state, word address) {
	word value = state->memory[address];
	return value;
}
