#include "ecall.h"
#include "debug.h"
ecall_callback_t ecall_callback;

void set_ecall_callback(ecall_callback_t callback) {
	ecall_callback = callback;
}

void ecall(State* state, word* instruction) {
	PRINT_DEBUG("ecall\n");
	ecall_callback(state);
}