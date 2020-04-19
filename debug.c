#include "debug.h"
#include "config.h"
#include "state.h"
#include <stdarg.h>
#include <stdio.h>

void print_debug(const char* fmt, ...) {
#ifndef PRINT_OPCODES_ALWAYS
	if (print_verbose == 0)
		return;
#endif
	
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}

