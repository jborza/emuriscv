#pragma once
#include "types.h"
#include "specifications.h"
typedef struct State {
	word pc;
	word x[REGISTERS];
	byte* memory;
} State;