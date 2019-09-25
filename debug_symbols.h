#pragma once
#include "types.h"
typedef struct symbol symbol;

typedef struct symbol {
	word offset;
	char* name;
	symbol* next;
} symbol;

symbol* symbol_list;

symbol* add_symbol(symbol* tail, word offset, char* name);