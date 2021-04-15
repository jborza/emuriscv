#pragma once
#include "memory.h"

#define LOW_RAM_SIZE   0x00010000 /* 64KB */
#define RAM_BASE_ADDR  0x80000000 

#define CLINT_BASE_ADDR 0x02000000
#define CLINT_SIZE      0x000c0000 

#define HTIF_BASE_ADDR 0x40008000 
#define HTIF_SIZE		16

#define UART_BASE_ADDR 0x10000000 
#define UART_SIZE		0x100


typedef struct RiscVMachine {
	MemoryMap* mem_map;
	int max_xlen;
	uint32_t ram_size;
	uint64_t htif_tohost, htif_fromhost;
	uint64_t timecmp;
	uint64_t rtc_start_time;
	uint64_t cycles;
} RiscVMachine;
