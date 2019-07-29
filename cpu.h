#pragma once
#include "state.h"
void emulate_op(State* state);
typedef void (*ecall_callback_t)(State* state);
void set_ecall_callback(ecall_callback_t callback);