#include "cpu_exception.h"
#include "csr.h"
#include "riscv_status.h"
#include "debug.h"

#define get_field(reg, mask) (((reg) & (mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(mask)) | (((val) * ((mask) & ~((mask) << 1))) & (mask)))

void raise_exception(State* state, word cause, word tval) {
	PRINT_DEBUG("taking trap: pc:%08x cause:%08x tval:%08x\n", state->pc, cause, tval);
	//3.1.13 In  systems  with  all  three  privilege  modes  (M/S/U), setting a bit in medeleg or mideleg will delegate the corresponding trap in S-mode or U-mode to the S-mode trap handler.
	//medeleg = machine exception delegation register
	//mideleg = machine interrupt delegation register

	//medeleg has a bit position allocated for every synchronous exception shown in Table 3.6 [cause.h], 
	//with the index of the bit position equal to the value returned in the mcause register
	word bit = cause;
	//interrupt bit is the high bit

	int interrupt = (bit & ((word)1 << (XLEN - 1))) != 0;
	word deleg = read_csr(state, interrupt ? CSR_MIDELEG : CSR_MEDELEG);

	if (interrupt)
		bit &= ~((word)1 << (XLEN - 1));

	int delegate_to_s_mode = bit < XLEN && ((deleg >> bit) & 1);
	PRINT_DEBUG("       trap: interrupt:%x deleg:%x delegate_to_s_mode:%x\n", interrupt, deleg, delegate_to_s_mode);

	//handle 
	if (state->privilege <= PRIV_S && delegate_to_s_mode) {
		//sepc
		write_csr(state, CSR_SEPC, state->pc - 4); //pc was pre-incremented in emulate_op
		//secause
		write_csr(state, CSR_SCAUSE, cause);
		//stval
		write_csr(state, CSR_STVAL, tval);
		//mstatus update
		//set priv to S - why?
		//update PC
		state->pc = read_csr(state, CSR_STVEC);

		word s = state->status;
		s = set_field(s, MSTATUS_SPIE, get_field(s, MSTATUS_SIE));
		s = set_field(s, MSTATUS_SPP, state->privilege);
		s = set_field(s, MSTATUS_SIE, 0);
		write_csr(state, CSR_MSTATUS, s);
		state->privilege = PRIV_S;
	}
	else { //handle in machine mode
		//mcause
		write_csr(state, CSR_MCAUSE, cause);
		//mepc
		write_csr(state, CSR_MEPC, state->pc - 4); //pc was pre-incremented in emulate_op
		//mtval
		write_csr(state, CSR_MTVAL, tval);

		state->pc = read_csr(state, CSR_MTVEC);

		word s = state->status;
		s = set_field(s, MSTATUS_MPIE, get_field(s, MSTATUS_MIE));
		s = set_field(s, MSTATUS_MPP, state->privilege);
		s = set_field(s, MSTATUS_MIE, 0);
		write_csr(state, CSR_MSTATUS, s);
		//set_priv to M mode
		state->privilege = PRIV_M;
	}
	//clear pending
	state->pending_exception = 0;
	state->pending_tval = 0;
}