#pragma once
#include "types.h"
#include "specifications.h"
#include "memory_map.h"
#include "cause.h"	

#define PRIV_M 3
#define PRIV_RESERVED 2
#define PRIV_S 1
#define PRIV_U 0

#define PC_HISTORY_DEPTH 8

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
	word pending_tval;
	word pending_exception;
	int has_pending_exception;
	int privilege;
	word load_reservation;

	word pc_history[PC_HISTORY_DEPTH];
} State;