#include "cpu.h"
#include "memory.h"
#include "opcodes.h"
#include "instruction.h"
#include <stdio.h>
#include "debug.h"
#include "decode.h"

#define GET_RD(x) (x >> 7) & 0x1F
#define GET_RS1(x) (x >> 15) & 0x1F
#define GET_RS2(x) (x >> 20) & 0x1F

int decode_opcode(word * instruction) {
	//risc opcodes https://klatz.co/blog/riscv-opcodes
	InstructionAny* any = instruction;
	//get lower OPCODE_BITS bits
	return any->opcode;
}

word get_rs1_value(State* state, word* instruction) {
	return get_reg(state, GET_RS1(*instruction));
}

word get_rs2_value(State* state, word* instruction) {
	return get_reg(state, GET_RS2(*instruction));
}

sword get_rs1_signed_value(State* state, word* instruction) {
	return (sword)get_reg(state, GET_RS1(*instruction));
}

sword get_rs2_signed_value(State* state, word* instruction) {
	return (sword)get_reg(state, GET_RS2(*instruction));
}

int set_rd_value(State* state, word* instruction, word value) {
	return set_reg(state, GET_RD(*instruction), value);
}

int get_rd(word* instruction) {
	return (*instruction >> 7) & 0x1f;
}

int get_rs1(word* instruction) {
	return (*instruction >> 15) & 0x1f;
}

int get_rs2(word* instruction) {
	return (*instruction >> 20) & 0x1f;
}

int set_reg(State* state, int index, word value) {
	if (index == 0) {
		//illegal instruction;
		return -1;
	}
	state->x[index] = value;
	return index;
}

inline word get_reg(State* state, int index) {
	return state->x[index];
}

void add(State* state, word* instruction) {
	PRINT_DEBUG("add x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) + get_rs2_value(state, instruction);
	set_rd_value(state, instruction, value);
}

void addi(State* state, word* instruction) {
	sword imm = get_i_imm(*instruction);
	PRINT_DEBUG("addi x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), imm);
	word value =  get_rs1_value(state, instruction) + imm;
	set_rd_value(state, instruction, value);
}

//bitwise and between rs1 and rs2
void and (State* state, word* instruction) {
	PRINT_DEBUG("and x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) & get_rs2_value(state, instruction);
	set_rd_value(state, instruction, value);
}

//bitwise and on rs1 and sign-extended 12-bit immediate
void andi(State* state, word* instruction) {
	sword imm = get_i_imm(*instruction);
	PRINT_DEBUG("andi x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), imm);
	word value = get_rs1_value(state, instruction) & imm;
	set_rd_value(state, instruction, value);
}

void auipc(State* state, word* instruction) {
	InstructionU* in = instruction;
	PRINT_DEBUG("auipc x%d,0x%08x\n", GET_RD(*instruction), in->data << 12);
	word offset = in->data << 12;
	offset += state->pc - INSTRUCTION_LENGTH_BYTES; //note: it would have been nicer to increment PC after all other instructions instead of this ugly hack
	set_rd_value(state, instruction, offset);
}

void beq(State* state, word* instruction) {
	PRINT_DEBUG("beq x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_value(state, instruction) == get_rs2_value(state, instruction))
	{
		//set PC = PC + offset
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}

//branch on greater or equal
void bge(State* state, word* instruction) {
	PRINT_DEBUG("bge x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_signed_value(state, instruction) >= get_rs2_signed_value(state, instruction))
	{
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}

//branch on greater or equal unsigned
void bgeu(State* state, word* instruction) {
	PRINT_DEBUG("bgeu x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_value(state, instruction) >= get_rs2_value(state, instruction))
	{
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}

//branch on less than
void blt(State* state, word* instruction) {
	PRINT_DEBUG("blt x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_signed_value(state, instruction) < get_rs2_signed_value(state, instruction))
	{
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}

//branch on less than unsigned
void bltu(State* state, word* instruction) {
	PRINT_DEBUG("bltu x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_value(state, instruction) < get_rs2_value(state, instruction))
	{
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}

void bne(State* state, word* instruction) {
	//branch if src1 and src2 not equal
	PRINT_DEBUG("bne x%d,x%d,0x%08x\n", GET_RS1(*instruction), GET_RS2(*instruction), get_b_imm(*instruction));
	if (get_rs1_value(state, instruction) != get_rs2_value(state, instruction))
	{
		//set PC = PC + offset
		int offset = get_b_imm(*instruction);
		state->pc += offset - INSTRUCTION_LENGTH_BYTES;
	}
}


void fence(State* state, word* instruction) {
	PRINT_DEBUG("fence [no-op]\n");
}
void fencei(State* state, word* instruction) {
	PRINT_DEBUG("fencei [no-op]\n");
}

//jump and link, J-immediate, 
void jal(State* state, word* instruction) {
	//stores the address of the instruction following the jump (pc+4) into rd
	//J-immediate encoding

	//signed offset in multiples of 2 bytes
	sword offset = get_j_imm(*instruction);
	PRINT_DEBUG("jal x%d,0x%08x\n", GET_RD(*instruction), offset);	
	//set register if not x0 to the target
	word destination = state->pc + offset - INSTRUCTION_LENGTH_BYTES; 
	word return_address = state->pc; //next instruction address
	set_rd_value(state, instruction, return_address);
	state->pc = destination;
}

//jump and link register
void jalr(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("jalr x%d, x%d, 0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), offset);
	word destination = get_rs1_value(state, instruction) + offset;
	word return_address = state->pc;
	set_rd_value(state, instruction, return_address);
	state->pc = destination;
}

//load 8-bit sign-extended value from memory into rd
void lb(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lb x%d,%d(x%d)\n", GET_RD(*instruction), offset, GET_RS1(*instruction));
	word address = get_rs1_value(state, instruction) + offset;
	word value = read_byte_signed(state, address);
	set_rd_value(state, instruction, value);
}

//load 8-bit zero-extended value from memory into rd
void lbu(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lbu x%d,%d(x%d)\n", GET_RD(*instruction), offset, GET_RS1(*instruction));
	word address = get_rs1_value(state, instruction) + offset;
	word value = read_byte_unsigned(state, address);
	set_rd_value(state, instruction, value);
}

//load 16-bit sign-extended value from memory into rd
void lh(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lh x%d,%d(x%d)\n", GET_RD(*instruction), offset, GET_RS1(*instruction));
	word address = get_rs1_value(state, instruction) + offset;
	word value = read_halfword_signed(state, address);
	set_rd_value(state, instruction, value);
}

//load 16-bit zero-extended value from memory into rd
void lhu(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lhu x%d,%d(x%d)\n", GET_RD(*instruction), offset, GET_RS1(*instruction));
	word address = get_rs1_value(state, instruction) + offset;
	word value = read_halfword_unsigned(state, address);
	set_rd_value(state, instruction, value);
}

void lui(State* state, word* instruction) {
	InstructionU* in = instruction;
	PRINT_DEBUG("lui x%d,0x%08x\n", GET_RD(*instruction), in->data << 12);
	word value = in->data << 12;
	set_rd_value(state, instruction, value);
}

//load 32-bit value from memory into rd
void lw(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lw x%d,%d(x%d)\n", GET_RD(*instruction), offset, GET_RS1(*instruction));
	word address = get_rs1_value(state, instruction) + offset;
	word value = read_word(state, address);
	set_rd_value(state, instruction, value);
}

void or (State* state, word* instruction) {
	PRINT_DEBUG("or x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) | get_rs2_value(state, instruction);
	set_rd_value(state, instruction, value);
}

//bitwise or on rs1 and sign-extended 12-bit immediate
void ori(State* state, word* instruction) {
	InstructionI* in = instruction;
	PRINT_DEBUG("ori x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->imm);
	word value = get_rs1_value(state, instruction) | in->imm;
	set_rd_value(state, instruction, value);
}

//store 8-bit value from the low bits of rs2 to the memory
void sb(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sb x%d,%d(x%d)\n", GET_RS2(*instruction), offset, GET_RS1(*instruction));
	word value = get_rs2_value(state, instruction);
	word address = get_rs1_value(state, instruction) + offset;
	write_byte(state, address, value);
}

//store 16-bit value from the low bits of rs2 to the memory
void sh(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sh x%d,%d(x%d)\n", GET_RS2(*instruction), offset, GET_RS1(*instruction));
	word value = get_rs2_value(state, instruction);
	word address = get_rs1_value(state, instruction) + offset;
	write_halfword(state, address, value);
}

void sll(State* state, word* instruction) {
	PRINT_DEBUG("sll x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	unsigned int shamt = get_rs2_value(state, instruction) & 0x1F; //take the lower 5 bits as a shift amount
	word value = get_rs1_value(state, instruction) << shamt;
	set_rd_value(state, instruction, value);
}

void slli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("slli x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->shamt);
	word value = get_reg(state, GET_RS1(*instruction)) << in->shamt;
	set_reg(state, GET_RD(*instruction), value);
}

void slt(State* state, word* instruction) {
	//signed comparison, if rs1 < rs2 then rd=1 else rd=0
	PRINT_DEBUG("slt x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = (sword)get_rs1_value(state, instruction) < (sword)get_rs2_value(state, instruction) ? 1 : 0; 
	set_rd_value(state, instruction, value);
}

void sltu(State* state, word* instruction) {
	//unsigned comparison, if rs1 < rs2 then rd=1 else rd=0
	PRINT_DEBUG("sltu x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) < get_rs2_value(state, instruction) ? 1 : 0; 
	set_rd_value(state, instruction, value);
}

//set less than immediate
void slti(State* state, word* instruction) {
	//places the value 1 in register rd if register rs1 is less than the sign - extended immediate when both are treated as signed numbers, else 0 is written to rd
	InstructionI* in = instruction;
	PRINT_DEBUG("slti x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->imm);
	word value = (sword)get_rs1_value(state, instruction) < in->imm;
	set_rd_value(state, instruction, value);
}

//set less than immediate unsigned
void sltiu(State* state, word* instruction) {
	//places the value 1 in register rd if register rs1 is less than the sign - extended immediate when both are treated as signed numbers, else 0 is written to rd
	InstructionI* in = instruction;
	PRINT_DEBUG("sltiu x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->imm);
	word value = get_rs1_value(state, instruction) < in->imm;
	set_rd_value(state, instruction, value);
}

//shift right arithmetic
void sra(State* state, word* instruction) {
	PRINT_DEBUG("sra x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	unsigned int shamt = get_rs2_value(state, instruction) & 0x1F; //take the lower 5 bits as a shift amount
	word value = (sword)get_rs1_value(state, instruction) >> shamt;
	set_rd_value(state, instruction, value);
}

void srai(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("srai x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->shamt);
	word value = (sword)get_rs1_value(state,instruction) >> in->shamt;
	set_rd_value(state, instruction, value);
}

//shift right logical
void srl(State* state, word* instruction) {
	PRINT_DEBUG("sll x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	unsigned int shamt = get_rs2_value(state, instruction) & 0x1F; //take the lower 5 bits as a shift amount
	word value = get_rs1_value(state, instruction) >> shamt;
	set_rd_value(state, instruction, value);
}

void srli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("srli x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->shamt);
	word value = get_rs1_value(state, instruction) >> in->shamt;
	set_rd_value(state, instruction, value);
}

void sub(State* state, word* instruction) {
	PRINT_DEBUG("sub x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) - get_rs2_value(state, instruction);
	set_rd_value(state, instruction, value);
}

//store 32-bit value from rs to memory
void sw(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sw x%d,%d(x%d)\n", GET_RS2(*instruction), offset, GET_RS1(*instruction));
	word value = get_rs2_value(state, instruction);
	word address = get_rs1_value(state, instruction) + offset;
	write_word(state, address, value);
}

void xor (State* state, word* instruction) {
	PRINT_DEBUG("xor x%d,x%d,x%d\n", GET_RD(*instruction), GET_RS1(*instruction), GET_RS2(*instruction));
	word value = get_rs1_value(state, instruction) ^ get_rs2_value(state, instruction);
	set_rd_value(state, instruction, value);
}

//bitwise xor on rs1 and sign-extended 12-bit immediate
void xori(State* state, word* instruction) {
	InstructionI* in = instruction;
	PRINT_DEBUG("xori x%d,x%d,0x%08x\n", GET_RD(*instruction), GET_RS1(*instruction), in->imm);
	word value = get_rs1_value(state, instruction) ^ in->imm;
	set_rd_value(state, instruction, value);
}

void emulate_op(State* state) {
	word* instruction = fetch_next_word(state);
	//state->pc += INSTRUCTION_LENGTH_BYTES;
	if ((*instruction & MASK_ADD) == MATCH_ADD) {
		add(state, instruction);
	}
	else if ((*instruction & MASK_ADDI) == MATCH_ADDI) {
		addi(state, instruction);
	}
	else if ((*instruction & MASK_AND) == MATCH_AND) {
		and (state, instruction);
	}
	else if ((*instruction & MASK_ANDI) == MATCH_ANDI) {
		andi(state, instruction);
	}
	else if ((*instruction & MASK_AUIPC) == MATCH_AUIPC) {
		auipc(state, instruction);
	}
	else if ((*instruction & MASK_BEQ) == MATCH_BEQ) {
		beq(state, instruction);
	}
	else if ((*instruction & MASK_BGE) == MATCH_BGE) {
		bge(state, instruction);
	}
	else if ((*instruction & MASK_BGEU) == MATCH_BGEU) {
		bgeu(state, instruction);
	}
	else if ((*instruction & MASK_BLT) == MATCH_BLT) {
		blt(state, instruction);
	}
	else if ((*instruction & MASK_BLTU) == MATCH_BLTU) {
		bltu(state, instruction);
	}
	else if ((*instruction & MASK_BNE) == MATCH_BNE) {
		bne(state, instruction);
	}
	else if ((*instruction & MASK_EBREAK) == MATCH_EBREAK) {
		ebreak(state, instruction);
	}
	else if ((*instruction & MASK_ECALL) == MATCH_ECALL) {
		ecall(state, instruction);
	}
	else if ((*instruction & MASK_FENCE) == MATCH_FENCE) {
		fence(state, instruction);
	}
	else if ((*instruction & MASK_FENCE_I) == MATCH_FENCE_I) {
		fencei(state, instruction);
	}
	else if ((*instruction & MASK_LUI) == MATCH_LUI) {
		lui(state, instruction);
	}
	else if ((*instruction & MASK_JAL) == MATCH_JAL) {
		jal(state, instruction);
	}
	else if ((*instruction & MASK_JALR) == MATCH_JALR) {
		jalr(state, instruction);
	}
	else if ((*instruction & MASK_LB) == MATCH_LB) {
		lb(state, instruction);
	}
	else if ((*instruction & MASK_LBU) == MATCH_LBU) {
		lbu(state, instruction);
	}
	else if ((*instruction & MASK_LH) == MATCH_LH) {
		lh(state, instruction);
	}
	else if ((*instruction & MASK_LHU) == MATCH_LHU) {
		lhu(state, instruction);
	}
	else if ((*instruction & MASK_LUI) == MATCH_LUI) {
		lui(state, instruction);
	}
	else if ((*instruction & MASK_LW) == MATCH_LW) {
		lw(state, instruction);
	}
	else if ((*instruction & MASK_OR) == MATCH_OR) {
		or (state, instruction);
	}
	else if ((*instruction & MASK_ORI) == MATCH_ORI) {
		ori(state, instruction);
	}
	else if ((*instruction & MASK_SB) == MATCH_SB) {
		sb(state, instruction);
	}
	else if ((*instruction & MASK_SH) == MATCH_SH) {
		sh(state, instruction);
	}
	else if ((*instruction & MASK_SLL) == MATCH_SLL) {
		sll(state, instruction);
	}
	else if ((*instruction & MASK_SLLI) == MATCH_SLLI) {
		slli(state, instruction);
	}
	else if ((*instruction & MASK_SLT) == MATCH_SLT) {
		slt(state, instruction);
	}
	else if ((*instruction & MASK_SLTI) == MATCH_SLTI) {
		slti(state, instruction);
	}
	else if ((*instruction & MASK_SLTIU) == MATCH_SLTIU) {
		sltiu(state, instruction);
	}
	else if ((*instruction & MASK_SLTU) == MATCH_SLTU) {
		sltu(state, instruction);
	}
	else if ((*instruction & MASK_SRA) == MATCH_SRA) {
		sra(state, instruction);
	}
	else if ((*instruction & MASK_SRAI) == MATCH_SRAI) {
		srai(state, instruction);
	}
	else if ((*instruction & MASK_SRL) == MATCH_SRL) {
		srl(state, instruction);
	}
	else if ((*instruction & MASK_SRLI) == MATCH_SRLI) {
		srli(state, instruction);
	}
	else if ((*instruction & MASK_SUB) == MATCH_SUB) {
		sub(state, instruction);
	}
	else if ((*instruction & MASK_SW) == MATCH_SW) {
		sw(state, instruction);
	}
	else if ((*instruction & MASK_XOR) == MATCH_XOR) {
		xor (state, instruction);
	}
	else if ((*instruction & MASK_XORI) == MATCH_XORI) {
		xori(state, instruction);
	}
	else {
		printf("Unknown instruction: %8X ", *instruction);
		return;
	}
}