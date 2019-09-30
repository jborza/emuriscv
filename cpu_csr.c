#include "cpu_csr.h"
#include "debug.h"
#include "cpu.h"
#include "csr.h"

//atomic read/write CSR
void csrrw(State* state, word* instruction) {
	word csr = get_i_imm_unsigned(*instruction);
	PRINT_DEBUG("csrrw x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), csr);

	//do not read CSR if rd is x0
	if (GET_RD(*instruction) != 0)
	{
		//read old value of CSR, zero-extend to XLEN bits, write to rd
		word value = read_csr(state, csr);
		//these following two sets are atomic
		set_rd_value(state, instruction, value);
	}
	//write rs1 to csr
	write_csr(state, csr, get_rs1_value(state, instruction));
}

//atomic read & clear bits in CSR
void csrrc(State* state, word* instruction) {
	//read old value of CSR, zero-extend to XLEN bits, write to rd
	//read value from rs1, use as bit mask to clear bits in CSR
	word csr = get_i_imm_unsigned(*instruction);
	PRINT_DEBUG("csrrc x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), csr);
	//do not read CSR if rd is x0
	if (GET_RD(*instruction) != 0)
	{
		set_rd_value(state, instruction, read_csr(state, csr));
	}
	//any bit that is high in rs1 will cause the correspoding bit to be cleared in CSR
	word value = get_rs1_value(state, instruction);
	word csr_value = read_csr(state, csr) & ~value;
	write_csr(state, csr, csr_value);
}

//instead of rs1 use a zero-extended 5-bit unsigned immediate
void csrrwi(State* state, word* instruction) {
	word csr = get_i_imm_unsigned(*instruction);
	PRINT_DEBUG("csrrwi x%d,0x%x,0x%08x\n", GET_RD(*instruction), GET_CSR_IMM(*instruction), csr);
	//TODO nice implementation
	//do not read CSR if rd is x0
	if (GET_RD(*instruction) != 0)
	{
		set_rd_value(state, instruction, read_csr(state, csr));
	}
	//any bit that is high in rs1 will cause the correspoding bit to be cleared in CSR
	word value = GET_CSR_IMM(*instruction);
	word csr_value = read_csr(state, csr) & ~value;
	write_csr(state, csr, csr_value);
}

void csrrsi(State* state, word* instruction) {
	//TODO verify implementation
	word csr = get_i_imm_unsigned(*instruction);
	PRINT_DEBUG("csrrsi x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_CSR_IMM(*instruction), csr);
	//read old value of CSR, zero-extend to XLEN bits, write to rd
	set_rd_value(state, instruction, state->csr[csr]);

	//switch (csr) {
	//case 0xf14: //mhardid
	//	set_rd_value(state, instruction, state->mhartid);
	//	break;
	//}
	//any bit that is high in rs1 will cause the correspoding bit to be set in CSR
	word value = GET_CSR_IMM(*instruction);// get_rs1_value(state, instruction);

	//if the immediate value is zero, then do not write to CSR
	if (value != 0) {

		word csr_value = read_csr(state, csr) | value;
		write_csr(state, csr, csr_value);
	}
}

void csrrci(State* state, word* instruction) {
	//TODO nicer implementation
	//read old value of CSR, zero-extend to XLEN bits, write to rd
	//read value from rs1, use as bit mask to clear bits in CSR
	word csr = get_i_imm_unsigned(*instruction);
	PRINT_DEBUG("csrrci x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_CSR_IMM(*instruction), csr);
	//do not read CSR if rd is x0
	if (GET_RD(*instruction) != 0)
	{
		set_rd_value(state, instruction, read_csr(state, csr));
	}
	//any bit that is high in rs1 will cause the correspoding bit to be cleared in CSR
	//if the immediate value is zero, then do not write to CSR
	word value = GET_CSR_IMM(*instruction);
	if (value != 0) {
		word csr_value = read_csr(state, csr) & ~value;
		write_csr(state, csr, csr_value);
	}
}