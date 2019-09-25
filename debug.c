#include "debug.h"
#include "config.h"
#include <stdarg.h>
void print_debug(const char* fmt, ...) {
	if (print_verbose == 0)
		return;
	
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);

	/*va_list vl;
	
	va_start(vl, s);
	printf(s, vl);
	va_end(vl);*/
}