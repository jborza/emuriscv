#pragma once
#include "config.h"

#ifdef PRINT_OPCODES
#include <stdio.h>
#define PRINT_DEBUG(...) printf(__VA_ARGS__)
#else
#define PRINT_DEBUG(...) // no-op
#endif