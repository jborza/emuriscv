#include "memory.h"

word* fetch_next_word(State* state) {
	word* address = state->memory + state->pc;
	state->pc += 4;
	return address;
}