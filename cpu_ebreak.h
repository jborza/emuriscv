#pragma once
#include "state.h"
#ifdef _DEBUG
#include <signal.h>
#endif

void ebreak(State* state, word* instruction);