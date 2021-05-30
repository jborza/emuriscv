#include "cpu_a.h"
#include "cpu.h"
#include "memory.h"
#include "debug.h"


/*
The atomic memory operation (AMO) instructions perform read-modify-write operations for multiprocessor synchronization
and are encoded with an R-type instruction format.

These AMO in-structions atomically load a data value from the address in *rs1*,
place the value into register *rd*, apply a binary operator to the loaded value and the original value in *rs2*,
then store the result back to the address in *rs1*.

AMOs can either operate on 64-bit (RV64 only) or 32-bit words in memory.
For RV64, 32-bit AMOs always sign-extend the value placed in rd*/

#define AMO_OP_W(NAME, OP) { \
PRINT_DEBUG(NAME" x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction)); \
word address = get_rs1_value(state, instruction); \
word value = read_word(state, address); \
word original_value = value; \
value = value OP get_rs2_value(state, instruction); \
write_word(state, address, value); \
set_rd_value(state, instruction, original_value); \
}

void amoadd_w(State* state, word* instruction) {
	AMO_OP_W("amoadd.w", +)
}

void amoand_w(State* state, word* instruction) {
	AMO_OP_W("amoand.w", &)
}

void amoor_w(State* state, word* instruction) {
	AMO_OP_W("amoor.w", | )
}

void amoxor_w(State* state, word* instruction) {
	AMO_OP_W("amoxor.w", ^);
}

void amoswap_w(State* state, word* instruction) {
	PRINT_DEBUG("amoswap.w x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	//load a data value from the address in *rs1*
	word address = get_rs1_value(state, instruction);
	word value = read_word(state, address);
	//place the value into register *rd*
	set_rd_value(state, instruction, value);

	//apply a binary operator to the loaded value and the original value in *rs2* (swap?)
	value = get_rs2_value(state, instruction);

	//then store the result back to the address in *rs1* .
	write_word(state, address, value);
}

void lr(State* state, word* instruction) {
	PRINT_DEBUG("lr x%d, (x%d)\n", GET_RD(*instruction), GET_RS1(*instruction));
	//loads word from address in rs1
	word address = get_rs1_value(state, instruction);
	word value = read_word(state, address);
	//places sign-extended value in rd
	set_rd_value(state, instruction, value);
	//registers a reservation on the address
	state->load_reservation = address;
}

void sc(State* state, word* instruction)
{
	// SC.W writes zero to rd on success or a nonzero code on failure.
	//see https://riscv.org/wp-content/uploads/2019/06/riscv-spec.pdf
	PRINT_DEBUG("sc x%d,x%d,(x%d)\n", GET_RD(*instruction), GET_RS2(*instruction), GET_RS1(*instruction));
	//writes a word in rs2 to the address in rs1 if reservation still exists on the address
	word address = get_rs1_value(state, instruction);
	word value = get_rs2_value(state, instruction);
	if (state->load_reservation == address) {
		write_word(state, address, value);
		//write zero to rd on success
		set_rd_value(state, instruction, 0);
	}
	else {
		//1 is the failure code
		set_rd_value(state, instruction, 1);
	}
	//clear the reservation after any sc operation
	state->load_reservation = -1;
}