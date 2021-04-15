#include "cpu_ecall.h"
#include "debug.h"
ecall_callback_t ecall_callback;

void set_ecall_callback(ecall_callback_t callback) {
	ecall_callback = callback;
}

void ecall(State* state, word* instruction) {
#ifdef RUN_TESTS //plain ecalls in test mode
	ecall_callback(state);
#else
	PRINT_DEBUG("ecall\n");
	if (state->privilege == PRIV_M) {
		state->pending_exception = CAUSE_MACHINE_ECALL;
	}
	else if (state->privilege == PRIV_S) {
		state->pending_exception = CAUSE_SUPERVISOR_ECALL;
	}
	else if (state->privilege == PRIV_U) {
		state->pending_exception = CAUSE_USER_ECALL;
	}
	raise_exception(state, state->pending_exception, state->pending_tval);
#endif
	//
}