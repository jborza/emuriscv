#pragma once
#include "state.h"

typedef void (*ecall_callback_t)(State* state);
void set_ecall_callback(ecall_callback_t callback);
void ecall(State* state, word* instruction);