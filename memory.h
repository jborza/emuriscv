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

word* fetch_next_word(State* state);

word read_word(State* state, word address);

word read_halfword_signed(State* state, word address);

word read_halfword_unsigned(State* state, word address);

word read_byte_signed(State* state, word address);

word read_byte_unsigned(State* state, word address);

void write_word(State* state, word address, word value);

void write_halfword(State* state, word address, halfword value);

void write_byte(State* state, word address, byte value);

enum access_type {
	LOAD,
	STORE,
	FETCH,
};

typedef struct MemoryTarget {
	MemoryRange* range;
	uint8_t* ptr;
} MemoryTarget;

int get_memory_target(State* state, word virtual_address, enum access_type access_type, MemoryTarget* target);

/*
 * PTE format:
 * | XLEN-1  10 | 9             8 | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0
 *       PFN      reserved for SW   D   A   G   U   X   W   R   V
 */

#define _PAGE_ACCESSED_OFFSET 6

#define _PAGE_PRESENT   (1 << 0)
#define _PAGE_READ      (1 << 1)    /* Readable */
#define _PAGE_WRITE     (1 << 2)    /* Writable */
#define _PAGE_EXEC      (1 << 3)    /* Executable */
#define _PAGE_USER      (1 << 4)    /* User */
#define _PAGE_GLOBAL    (1 << 5)    /* Global */
#define _PAGE_ACCESSED  (1 << 6)    /* Set by hardware on any access */
#define _PAGE_DIRTY     (1 << 7)    /* Set by hardware on any write */
#define _PAGE_SOFT      (1 << 8)    /* Reserved for software */

#define ENABLE_DIRTY