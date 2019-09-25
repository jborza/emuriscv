#pragma once
#include "config.h"
#include "types.h"
#include "state.h"

/* "A" Standard Extension for AtomicInstructions, Version 2.0 */

void amoadd_w(State* state, word* instruction);
