#include "csr.h"
#include "stdio.h"

#define PG_SHIFT 12
#define PG_MASK ((1 << PG_SHIFT) - 1) 

word read_csr(State *state, int csr) {
	if (csr == CSR_SATP) {
		printf("reading CSR satp 0x%x\n", state->csr[csr]);
	}
	return state->csr[csr];
}
void write_csr(State *state, int csr, word value) {
	state->csr[csr] = value;
	if (csr == CSR_SATP) {
		printf("Setting CSR 0x%x to 0x%x\n", csr, value);
		int mode = value >> 31;
		int pte_addr_bits = 22;
		int pte_addr = (value & (((uint32_t)1 << pte_addr_bits) - 1)) << PG_SHIFT;
		printf("MODE:%d pte_addr:0x%x\n", mode, pte_addr);
	}
}