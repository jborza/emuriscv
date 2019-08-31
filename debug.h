#pragma once
#ifdef _DEBUG
#include <stdio.h>
#define PRINT_DEBUG(...) printf(__VA_ARGS__)
#else
#define PRINT_DEBUG(...) // no-op
#endif