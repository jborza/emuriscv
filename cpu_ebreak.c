#include "cpu_ebreak.h"
#include "debug.h"
#include <stdio.h>

void ebreak(State* state, word* instruction) {
	PRINT_DEBUG("ebreak\n");
#ifdef _DEBUG
	raise(SIGINT); //TODO or other reliable "invoke debugger" call
#endif
}