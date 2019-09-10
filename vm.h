#pragma once
#include "memory.h"

#define LOW_RAM_SIZE   0x00010000 /* 64KB */
#define RAM_BASE_ADDR  0x80000000 

typedef struct RiscVMachine {
	MemoryMap* mem_map;
	int max_xlen;
	uint32_t ram_size;
} RiscVMachine;
