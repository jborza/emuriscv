#include "csr.h"
#include "stdio.h"
#include "memory.h" //TODO remove

#define PG_SHIFT 12
#define PG_MASK ((1 << PG_SHIFT) - 1) 

word read_csr(State *state, int csr) {
	return state->csr[csr];
}

void write_csr(State *state, int csr, word value) {	
	state->csr[csr] = value;
}