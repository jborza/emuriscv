#include "cpu.h"
#include "csr.h"
#include "riscv_status.h"
#include "cpu_prv.h"
#include "debug.h"

//privileged
//return from machine-mode trap
void mret(State* state, word* instruction) {
	//an xRET instruction can be executed in privilege mode x or higher, where executing a lower privilege
	//xRET instruction will pop the relevant lower-privilege interrupt enable and privilege mode stack.

	//The MRET, HRET, SRET, or URET instructions are used to return from traps in M - mode, H - mode, S - mode, or U - mode respectively.When executing an xRET instruction, supposing xPP holds the value y, yIE is set to xPIE; the privilege mode is changed to y; xPIE is set to 1;and xPP is set to U.

	PRINT_DEBUG("mret\n");
	if (state->privilege != PRIV_M)
	{
		printf("Illegal instruction: MRET called out of machine mode\n");
		exit(1);
	}
	//jump back to mepc
	state->pc = read_csr(state, CSR_MEPC);
	word s = read_csr(state, CSR_MSTATUS);
	word prev_prv = get_field(s, MSTATUS_MPP);
	s = set_field(s, MSTATUS_MIE, get_field(s, MSTATUS_MPIE));
	s = set_field(s, MSTATUS_MPIE, 1);
	//set mpp to user privilege
	s = set_field(s, MSTATUS_MPP, PRIV_U);
	state->privilege = prev_prv;
	write_csr(state, CSR_MSTATUS, s);

}

//privileged
//return from supervisor-mode trap
void sret(State* state, word* instruction) {
	PRINT_DEBUG("sret\n");
	//TODO raise an illegal instruction operation when TSR=1 in mstatus
	//as per require_privilege(get_field(STATE.mstatus, MSTATUS_TSR) ? PRV_M : PRV_S);
	
	state->pc = read_csr(state, CSR_SEPC);

	word status = read_csr(state, CSR_MSTATUS);
	word previous_privilege = get_field(status, MSTATUS_SPP);
	//set interrupt enabled status from the previous interrupt enabled
	status = set_field(status, MSTATUS_SIE, get_field(status, MSTATUS_SPIE));
	//set previous interrupt enabled to 1
	status = set_field(status, MSTATUS_SPIE, 1);
	//set spp to user privilege
	status = set_field(status, MSTATUS_SPP, PRIV_U);
	state->privilege = previous_privilege;
	write_csr(state, CSR_MSTATUS, status);
}

//privileged
//return from user-mode trap
void uret(State* state, word* instruction) {
	PRINT_DEBUG("uret\n");
	//no-op

	//jump to uepc
	state->pc = read_csr(state, CSR_UEPC);
	//not really implementing this..
}

void sfence_vma(State* state, word* instruction) {
	//The supervisor memory - management fence instruction SFENCE.VMA is used to synchronize up - dates to in - memory memory - management data structures with current execution.
	//Instruction execution causes implicit reads and writes to these data structures; 
	//however, these implicit references are ordinarily not ordered with respect to loads and stores in the instruction stream.
	//Executing an  SFENCE.VMA  instruction  guarantees  that  any  stores  in  the  instruction  stream  prior  to  the SFENCE.VMA are ordered 
	//before all implicit references subsequent to the SFENCE.VMA.
	PRINT_DEBUG("sfence.vma\r\n");
	//no-op as there is no multi-core support
}

void wfi(State* state, word* instruction) {
	//wait for interrupt
	PRINT_DEBUG("wfi\n");
	//no-op
	//see how many instructions we did and exit
	printf("WFI\r\n");
	printf("executed %d instructions\r\n", state->instruction_counter);

	exit(1);
}