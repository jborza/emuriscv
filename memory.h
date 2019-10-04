#pragma once
#include "state.h"

#define SIZE_BYTE 0
#define SIZE_HALF 1
#define SIZE_WORD 2

#define SATP_MODE_BARE 0
#define SATP_MODE_SV32 1

#define PTE_ADDRESS_BITS 22

#define PGDIR_SHIFT 22
#define PTRS_PER_PGD 400

#define PAGE_SHIFT	(12)
#define PAGE_SIZE	((1UL) << PAGE_SHIFT)
#define PAGE_MASK	(~(PAGE_SIZE - 1))

#define PTE_V_MASK (1 << 0)

//the first address of the first page of memory
#define PAGE_OFFSET 0xc0000000

#define TRANSLATE_OK 0
#define PAGE_FAULT -1

#define CAUSE_FETCH_PAGE_FAULT    0xc
#define CAUSE_LOAD_PAGE_FAULT     0xd
#define CAUSE_STORE_PAGE_FAULT    0xf

word* fetch_next_word(State* state);

word read_word(State* state, word address);

word read_halfword_signed(State* state, word address);

word read_halfword_unsigned(State* state, word address);

word read_byte_signed(State* state, word address);

word read_byte_unsigned(State* state, word address);

void write_word(State* state, word address, word value);

void write_halfword(State* state, word address, halfword value);

void write_byte(State* state, word address, byte value);

typedef struct MemoryTarget {
	MemoryRange* range;
	uint8_t* ptr;
} MemoryTarget;

MemoryTarget get_memory_target(State* state, word address);