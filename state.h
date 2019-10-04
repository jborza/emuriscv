#pragma once
#include "types.h"
#include "specifications.h"
#include "memory_map.h"

typedef enum Status {
	RUNNING,
	BREAKPOINT,
	EXIT_TERMINATION
} Status;

typedef struct State {
	word pc;
	word x[REGISTERS];
	word csr[CSR_REGISTERS];
	word mhartid;
	Status status;
	MemoryMap* memory_map;
	word instruction_counter;
	word satp;
	word pending_tval;
	word pending_exception;
} State;