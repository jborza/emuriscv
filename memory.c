#include "memory.h"

byte fetch_byte(State* state) {
	return state->memory[state->pc++];
}



word fetch_next_word(State* state) {

}