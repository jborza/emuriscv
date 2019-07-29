#pragma once
#include "types.h"
#include "specifications.h"

typedef enum Status {
	RUNNING,
	BREAKPOINT,
	EXIT_TERMINATION
} Status;

typedef struct State {
	word pc;
	word x[REGISTERS];
	byte* memory;
	Status status;
} State;