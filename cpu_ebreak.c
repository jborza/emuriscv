#include "cpu_ebreak.h"
#include "debug.h"
#include <stdio.h>
#include <intrin.h>

void ebreak(State* state, word* instruction) {
	PRINT_DEBUG("ebreak\n");
#ifdef _DEBUG
#ifdef _WIN32
	__debugbreak();
#elif
	raise(SIGINT); //TODO or other reliable "invoke debugger" call
#endif
#endif
}