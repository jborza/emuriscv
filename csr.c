#include "csr.h"
#include "stdio.h"
#include "memory.h" //TODO remove

#define PG_SHIFT 12
#define PG_MASK ((1 << PG_SHIFT) - 1) 

word read_csr(State *state, int csr) {
	if (csr == CSR_MHARTID) {
		return state->mhartid;
	}
	return state->csr[csr];
}
void write_csr(State *state, int csr, word value) {
	if (csr == CSR_SATP) {
		printf("Setting CSR 0x%x to 0x%x\n", csr, value);
		int mode = value >> 31;
		int pte_addr = (value & (((uint32_t)1 << PTE_ADDRESS_BITS) - 1)) << PG_SHIFT;
		printf("MODE:%d pte_addr:0x%x\n", mode, pte_addr);
	}
	state->csr[csr] = value;
}