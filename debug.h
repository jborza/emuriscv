#pragma once
#include "config.h"

#ifdef PRINT_OPCODES
#include <stdio.h>
void print_debug(const char* s, ...);
#define PRINT_DEBUG(...) print_debug(__VA_ARGS__)
#else
#define PRINT_DEBUG(...) // no-op
#endif