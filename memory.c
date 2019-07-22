#include "memory.h"

word* fetch_next_word(State* state) {
	word* address = state->memory + state->pc;
	state->memory += 4;
	return address;
}