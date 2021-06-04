#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "disassembler.h"
#include "encoding.h"
#include "../instruction.h"
#include "../decode.h"
#include "state.h"
#include "register.h"

#ifndef  WIN3
#pragma GCC diagnostic ignored "-Wincompatible-pointer-types"
#endif // ! WIN32

#define INS_MATCH(MASK,MATCH,HANDLER) else if ((*instruction & MASK) == MATCH) { HANDLER(state, instruction);	}
#define PRINT_DEBUG _print_debug

void _print_debug(const char* fmt, ...) {
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}


//int get_rs1(word* instruction) {
//	return (*instruction >> 15) & 0x1f;
//}
//
//int get_rs2(word* instruction) {
//	return (*instruction >> 20) & 0x1f;
//}

void _add(State* state, word* instruction) {
	PRINT_DEBUG("add\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

void _addi(State* state, word* instruction) {
	sword imm = get_i_imm(*instruction);
	if(GET_RS1(*instruction) == 0){
		if(GET_RD(*instruction) == 0 && imm == 0)
			PRINT_DEBUG("nop\n");
		else
			PRINT_DEBUG("li\t%s,%d\n", register_name[GET_RD(*instruction)], imm);
	}
	else if(imm == 0)
		PRINT_DEBUG("mv\t%s,%s\n",  register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)]);
	else
		PRINT_DEBUG("addi\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], imm);
}

//bitwise and between rs1 and rs2
void /**/_and (State* state, word* instruction) {
	PRINT_DEBUG("and\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

//bitwise and on rs1 and sign-extended 12-bit immediate
void _andi(State* state, word* instruction) {
	sword imm = get_i_imm(*instruction);
	PRINT_DEBUG("andi\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], imm);
}

void _auipc(State* state, word* instruction) {
	InstructionU* in = instruction;
	PRINT_DEBUG("auipc\t%s,0x%x\n", register_name[GET_RD(*instruction)], (in->data << 12 >> 12) & 0xfffff);
}

void _beq(State* state, word* instruction) {
	if(GET_RS2(*instruction) == 0)
		PRINT_DEBUG("beqz\t%s,0x%x\n", register_name[GET_RS1(*instruction)], get_b_imm(*instruction) + state->pc);
	else
		PRINT_DEBUG("beq\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction) + state->pc);
}

//branch on greater or equal
void _bge(State* state, word* instruction) {
	if(GET_RS2(*instruction)==0)
		PRINT_DEBUG("bgez\t%s,0x%x\n", register_name[GET_RS1(*instruction)], get_b_imm(*instruction)+state->pc);
	else if(GET_RS1(*instruction)==0)
		PRINT_DEBUG("blez\t%s,0x%x\n", register_name[GET_RS2(*instruction)], get_b_imm(*instruction) + state->pc);
	else
		PRINT_DEBUG("bge\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction)+state->pc);
}

//branch on greater or equal unsigned
void _bgeu(State* state, word* instruction) {
	PRINT_DEBUG("bgeu\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction)+state->pc);
}

//branch on less than
void _blt(State* state, word* instruction) {
	if(GET_RS2(*instruction)==0)
		PRINT_DEBUG("bltz\t%s,0x%x\n", register_name[GET_RS1(*instruction)], get_b_imm(*instruction)+state->pc);
	else if(GET_RS1(*instruction)==0)
		PRINT_DEBUG("bgtz\t%s,0x%x\n", register_name[GET_RS2(*instruction)], get_b_imm(*instruction) + state->pc);
	else
		PRINT_DEBUG("blt\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction)+state->pc);
}

//branch on less than unsigned
void _bltu(State* state, word* instruction) {
	PRINT_DEBUG("bltu\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction)+state->pc);
}

void _bne(State* state, word* instruction) {
	//branch if src1 and src2 not equal
	if(GET_RS2(*instruction)==0)
		PRINT_DEBUG("bnez\t%s,0x%x\n", register_name[GET_RS1(*instruction)], get_b_imm(*instruction) + state->pc);
	else
		PRINT_DEBUG("bne\t%s,%s,0x%x\n", register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)], get_b_imm(*instruction) + state->pc);
}

void _fence(State* state, word* instruction) {
	PRINT_DEBUG("fence\n");
}
void _fencei(State* state, word* instruction) {
	PRINT_DEBUG("fence.i\n");
}

//jump and link, J-immediate, 
void _jal(State* state, word* instruction) {
	//stores the address of the instruction following the jump (pc+4) into rd
	//J-immediate encoding

	//signed offset in multiples of 2 bytes
	sword offset = get_j_imm(*instruction);
	int rd = GET_RD(*instruction);
	if(rd == 0)
		PRINT_DEBUG("j\t0x%X\n", offset+state->pc);
	else
		PRINT_DEBUG("jal\t%s,0x%X\n", register_name[rd], offset+state->pc);
}

//jump and link register
void _jalr(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	if(offset == 0 && GET_RD(*instruction) == 0 && GET_RS1(*instruction) == 1)
		PRINT_DEBUG("ret\n");
	else if(offset == 0 && GET_RD(*instruction) == 1)
		PRINT_DEBUG("jalr\t%s\n",  register_name[GET_RS1(*instruction)]);
	else if(offset == 0 && GET_RD(*instruction) == 0)
		PRINT_DEBUG("jr\t%s\n", register_name[GET_RS1(*instruction)]);
	else if(GET_RD(*instruction) == GET_RS1(*instruction))
		PRINT_DEBUG("jalr\t%d(%s)\n", offset, register_name[GET_RS1(*instruction)]);
	else
		PRINT_DEBUG("jalr\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], offset);
}

//load 8-bit sign-extended value from memory into rd
void _lb(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lb\t%s,%d(%s)\n", register_name[GET_RD(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

//load 8-bit zero-extended value from memory into rd
void _lbu(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lbu\t%s,%d(%s)\n", register_name[GET_RD(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

//load 16-bit sign-extended value from memory into rd
void _lh(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lh\t%s,%d(%s)\n", register_name[GET_RD(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

//load 16-bit zero-extended value from memory into rd
void _lhu(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lhu\t%s,%d(%s)\n", register_name[GET_RD(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

void _lui(State* state, word* instruction) {
	InstructionU* in = instruction;
	PRINT_DEBUG("lui\t%s,0x%x\n", register_name[GET_RD(*instruction)], (in->data << 12 >> 12) & 0xfffff);
}

//load 32-bit value from memory into rd
void _lw(State* state, word* instruction) {
	sword offset = get_i_imm(*instruction);
	PRINT_DEBUG("lw\t%s,%d(%s)\n", register_name[GET_RD(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

void _or (State * state, word * instruction) {
	PRINT_DEBUG("or\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

//bitwise or on rs1 and sign-extended 12-bit immediate
void _ori(State* state, word* instruction) {
	InstructionI* in = instruction;
	PRINT_DEBUG("ori\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->imm);
}

//store 8-bit value from the low bits of rs2 to the memory
void _sb(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sb\t%s,%d(%s)\n", register_name[GET_RS2(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

//store 16-bit value from the low bits of rs2 to the memory
void _sh(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sh\t%s,%d(%s)\n", register_name[GET_RS2(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

void _sll(State* state, word* instruction) {
	PRINT_DEBUG("sll\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

void _slli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("slli\t%s,%s,0x%x\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->shamt & 0x1F);
}

void _slt(State* state, word* instruction) {
	//signed comparison, if rs1 < rs2 then rd=1 else rd=0
	if(GET_RS1(*instruction)==0)
		PRINT_DEBUG("sgtz\t%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS2(*instruction)]);
	else
		PRINT_DEBUG("slt\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

void _sltu(State* state, word* instruction) {
	//unsigned comparison, if rs1 < rs2 then rd=1 else rd=0
	if(GET_RS1(*instruction)==0)
		PRINT_DEBUG("snez\t%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS2(*instruction)]);
	else
		PRINT_DEBUG("sltu\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

//set less than immediate
void _slti(State* state, word* instruction) {
	//places the value 1 in register rd if register rs1 is less than the sign - extended immediate when both are treated as signed numbers, else 0 is written to rd
	InstructionI* in = instruction;
	PRINT_DEBUG("slti\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->imm);
}

//set less than immediate unsigned
void _sltiu(State* state, word* instruction) {
	//places the value 1 in register rd if register rs1 is less than the sign - extended immediate when both are treated as signed numbers, else 0 is written to rd
	InstructionI* in = instruction;
	if(in->imm==1)
		PRINT_DEBUG("seqz\t%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)]);
	else
		PRINT_DEBUG("sltiu\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->imm);
}

//shift right arithmetic
void _sra(State* state, word* instruction) {
	PRINT_DEBUG("sra\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

void _srai(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("srai\t%s,%s,0x%x\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->shamt & 0x1F);
}

//shift right logical
void _srl(State* state, word* instruction) {
	PRINT_DEBUG("srl\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

void _srli(State* state, word* instruction) {
	InstructionIShift* in = instruction;
	PRINT_DEBUG("srli\t%s,%s,0x%x\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->shamt & 0x1F);
}

void _sub(State* state, word* instruction) {
	if(GET_RS1(*instruction)==0)
		PRINT_DEBUG("neg\t%s,%s\n",register_name[GET_RD(*instruction)], register_name[GET_RS2(*instruction)]);
	else
		PRINT_DEBUG("sub\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

//store 32-bit value from rs to memory
void _sw(State* state, word* instruction) {
	sword offset = get_s_imm(*instruction);
	PRINT_DEBUG("sw\t%s,%d(%s)\n", register_name[GET_RS2(*instruction)], offset, register_name[GET_RS1(*instruction)]);
}

void /**/_xor (State* state, word* instruction) {
	PRINT_DEBUG("xor\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]);
}

//bitwise xor on rs1 and sign-extended 12-bit immediate
void _xori(State* state, word* instruction) {
	InstructionI* in = instruction;
	if(in->imm==-1)
		PRINT_DEBUG("not\t%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)]);
	else
		PRINT_DEBUG("xori\t%s,%s,%d\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], in->imm);
}

void _mret(State* state, word* instruction) {
	PRINT_DEBUG("mret\n");
}

void _sret(State* state, word* instruction) {
	PRINT_DEBUG("sret\n");
}

void _uret(State* state, word* instruction) {
	PRINT_DEBUG("uret\n");
}

void _sfence_vma(State* state, word* instruction) {
	PRINT_DEBUG("sfence.vma\r\n");
}

void _wfi(State* state, word* instruction) {
	//wait for interrupt
	PRINT_DEBUG("wfi\n");
}

/// M Extension


#define M_OP(NAME, OP) { \
	PRINT_DEBUG(NAME"\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]); \
}

void _mul(State* state, word* instruction) {
	M_OP("mul", mul32);
}

void _mulh(State* state, word* instruction) {
	M_OP("mulh", mulh32);
}
void _mulhu(State* state, word* instruction) {
	M_OP("mulhu", mulhu32);
}

void _mulhsu(State* state, word* instruction) {
	M_OP("mulhsu", mulhsu32);
}
void _rem(State* state, word* instruction) {
	M_OP("rem", rem32);
}
void _remu(State* state, word* instruction) {
	M_OP("remu", remu32);
}
void _op_div(State* state, word* instruction) {
	M_OP("div", div32);
}
void _divu(State* state, word* instruction) {
	M_OP("divu", divu32);
}

// ZICSR

#define M_OP(NAME, OP) { \
	PRINT_DEBUG(NAME"\t%s,%s,%s\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)], register_name[GET_RS2(*instruction)]); \
}

void print_csr(const char* name_full, const char* name_zero, word* instruction){
	word csr = get_i_imm_unsigned(*instruction);
	if(GET_RD(*instruction) == 0)
		PRINT_DEBUG("%s\t%s,%s\n", name_zero, csr_name(csr), register_name[GET_RS1(*instruction)]); 
	else
		PRINT_DEBUG("%s\t%s,%s,%s\n", name_full, register_name[GET_RD(*instruction)], csr_name(csr), register_name[GET_RS1(*instruction)]);	
}

//atomic read & set bits in CSR
void _csrrs(State* state, word* instruction) {
	if(GET_RS1(*instruction)==0){
		word csr = get_i_imm_unsigned(*instruction);
		PRINT_DEBUG("csrr\t%s,%s\n", register_name[GET_RD(*instruction)], csr_name(csr)); 
	}
	else
		print_csr("csrrs","csrs",instruction);
}

//atomic read/write CSR
void _csrrw(State* state, word* instruction) {
	print_csr("csrrw", "csrw", instruction);
}

//atomic read & clear bits in CSR
void _csrrc(State* state, word* instruction) {
	print_csr("csrrc", "csrc", instruction);
}

void print_csri(const char* name_full, const char* name_zero, word* instruction){
	word csr = get_i_imm_unsigned(*instruction);
	if(GET_RD(*instruction) == 0)
		PRINT_DEBUG("%s\t%s,%d\n", name_zero, csr_name(csr), GET_CSR_IMM(*instruction)); 
	else
		PRINT_DEBUG("%s\t%s,%s,%d\n", name_full, register_name[GET_RD(*instruction)], csr_name(csr),  GET_CSR_IMM(*instruction));	
}

//instead of rs1 use a zero-extended 5-bit unsigned immediate
void _csrrwi(State* state, word* instruction) {
	print_csri("csrwwi","csrwi", instruction);
}

void _csrrsi(State* state, word* instruction) {
	print_csri("csrrsi","csrsi", instruction);
}

void _csrrci(State* state, word* instruction) {
	print_csri("csrrci","csrci", instruction);
}

void _ebreak(State* state, word* instruction) {
	PRINT_DEBUG("ebreak\n");
}

void _ecall(State* state, word* instruction) {
	PRINT_DEBUG("ecall\n");
}

// A extension

#define AMO_OP_W(NAME, OP) { \
PRINT_DEBUG(NAME"\t%s,%s,(%s)\n", register_name[GET_RD(*instruction)], register_name[GET_RS2(*instruction)], register_name[GET_RS1(*instruction)]); \
}

void _amoadd_w(State* state, word* instruction) {
	AMO_OP_W("amoadd.w", +)
}

void _amoand_w(State* state, word* instruction) {
	AMO_OP_W("amoand.w", &)
}

void _amoor_w(State* state, word* instruction) {
	AMO_OP_W("amoor.w", | )
}

void _amoxor_w(State* state, word* instruction) {
	AMO_OP_W("amoxor.w", ^);
}

void _amoswap_w(State* state, word* instruction) {
	AMO_OP_W("amoswap.w", +);
}

void _lr(State* state, word* instruction) {
	PRINT_DEBUG("lr\t%s,(%s)\n", register_name[GET_RD(*instruction)], register_name[GET_RS1(*instruction)]);
}

void _sc(State* state, word* instruction)
{
	// SC.W writes zero to rd on success or a nonzero code on failure.
	//see https://riscv.org/wp-content/uploads/2019/06/riscv-spec.pdf
	PRINT_DEBUG("sc\t%s,%s,(%s)\n", register_name[GET_RD(*instruction)], register_name[GET_RS2(*instruction)], register_name[GET_RS1(*instruction)]);
}


void disassemble_op(word *instruction, word offset){
	State _state;
	_state.pc = offset;
	State *state = &_state;
//dummy condition as opcode handlers are 'else if'
	if (0 == 1) {
		;
	}
		INS_MATCH(MASK_ADD, MATCH_ADD, _add)
		INS_MATCH(MASK_ADDI, MATCH_ADDI, _addi)
		INS_MATCH(MASK_AND, MATCH_AND, _and)
		INS_MATCH(MASK_ANDI, MATCH_ANDI, _andi)
		INS_MATCH(MASK_AUIPC, MATCH_AUIPC, _auipc)
		INS_MATCH(MASK_BEQ, MATCH_BEQ, _beq)
		INS_MATCH(MASK_BGE, MATCH_BGE, _bge)
		INS_MATCH(MASK_BGEU, MATCH_BGEU, _bgeu)
		INS_MATCH(MASK_BLT, MATCH_BLT, _blt)
		INS_MATCH(MASK_BLTU, MATCH_BLTU, _bltu)
		INS_MATCH(MASK_BNE, MATCH_BNE, _bne)
		INS_MATCH(MASK_EBREAK, MATCH_EBREAK, _ebreak)
		INS_MATCH(MASK_ECALL, MATCH_ECALL, _ecall)
		INS_MATCH(MASK_FENCE, MATCH_FENCE, _fence)
		INS_MATCH(MASK_FENCE_I, MATCH_FENCE_I, _fencei)
		INS_MATCH(MASK_JAL, MATCH_JAL, _jal)
		INS_MATCH(MASK_JALR, MATCH_JALR, _jalr)
		INS_MATCH(MASK_LB, MATCH_LB, _lb)
		INS_MATCH(MASK_LBU, MATCH_LBU, _lbu)
		INS_MATCH(MASK_LH, MATCH_LH, _lh)
		INS_MATCH(MASK_LHU, MATCH_LHU, _lhu)
		INS_MATCH(MASK_LUI, MATCH_LUI, _lui)
		INS_MATCH(MASK_LW, MATCH_LW, _lw)
		INS_MATCH(MASK_OR, MATCH_OR, _or)
		INS_MATCH(MASK_ORI, MATCH_ORI, _ori)
		INS_MATCH(MASK_SB, MATCH_SB, _sb)
		INS_MATCH(MASK_SH, MATCH_SH, _sh)
		INS_MATCH(MASK_SLL, MATCH_SLL, _sll)
		INS_MATCH(MASK_SLLI, MATCH_SLLI, _slli)
		INS_MATCH(MASK_SLT, MATCH_SLT, _slt)
		INS_MATCH(MASK_SLTI, MATCH_SLTI, _slti)
		INS_MATCH(MASK_SLTIU, MATCH_SLTIU, _sltiu)
		INS_MATCH(MASK_SLTU, MATCH_SLTU, _sltu)
		INS_MATCH(MASK_SRA, MATCH_SRA, _sra)
		INS_MATCH(MASK_SRAI, MATCH_SRAI, _srai)
		INS_MATCH(MASK_SRL, MATCH_SRL, _srl)
		INS_MATCH(MASK_SRLI, MATCH_SRLI, _srli)
		INS_MATCH(MASK_SUB, MATCH_SUB, _sub)
		INS_MATCH(MASK_SW, MATCH_SW, _sw)
		INS_MATCH(MASK_XOR, MATCH_XOR, _xor)
		INS_MATCH(MASK_XORI, MATCH_XORI, _xori)

		INS_MATCH(MASK_WFI, MATCH_WFI, _wfi)
		INS_MATCH(MASK_MRET, MATCH_MRET, _mret)
		INS_MATCH(MASK_SRET, MATCH_SRET, _sret)
		INS_MATCH(MASK_URET, MATCH_URET, _uret)

#ifdef EXTENSION_ZICSR
		INS_MATCH(MASK_CSRRS, MATCH_CSRRS, _csrrs)
		INS_MATCH(MASK_CSRRW, MATCH_CSRRW, _csrrw)
		INS_MATCH(MASK_CSRRC, MATCH_CSRRC, _csrrc)
		INS_MATCH(MASK_CSRRSI, MATCH_CSRRSI, _csrrsi)
		INS_MATCH(MASK_CSRRWI, MATCH_CSRRWI, _csrrwi)
		INS_MATCH(MASK_CSRRCI, MATCH_CSRRCI, _csrrci)
#endif

#ifdef EXTENSION_A
		INS_MATCH(MASK_AMOADD_W, MATCH_AMOADD_W, _amoadd_w)
		INS_MATCH(MASK_AMOOR_W, MATCH_AMOOR_W, _amoor_w)
		INS_MATCH(MASK_AMOAND_W, MATCH_AMOAND_W, _amoand_w)
		INS_MATCH(MASK_AMOXOR_W, MATCH_AMOXOR_W, _amoxor_w)
		INS_MATCH(MASK_AMOSWAP_W, MATCH_AMOSWAP_W, _amoswap_w)
		INS_MATCH(MASK_LR_W, MATCH_LR_W, _lr)
		INS_MATCH(MASK_SC_W, MATCH_SC_W, _sc)
#endif
		INS_MATCH(MASK_SFENCE_VMA, MATCH_SFENCE_VMA, _sfence_vma)
#ifdef EXTENSION_M
		INS_MATCH(MASK_MUL, MATCH_MUL, _mul)
		INS_MATCH(MASK_MULH, MATCH_MULH, _mulh)
		INS_MATCH(MASK_MULHSU, MATCH_MULHSU, _mulhsu)
		INS_MATCH(MASK_MULHU, MATCH_MULHU, _mulhu)
		INS_MATCH(MASK_DIV, MATCH_DIV, _op_div)
		INS_MATCH(MASK_DIVU, MATCH_DIVU, _divu)
		INS_MATCH(MASK_REM, MATCH_REM, _rem)
		INS_MATCH(MASK_REMU, MATCH_REMU, _remu)
#endif
else {
		printf("Unknown instruction: %8X \n", *instruction);
}
}