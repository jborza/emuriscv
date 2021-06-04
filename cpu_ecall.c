#include "cpu_ecall.h"
#include "debug.h"
#include "debug_syscalls.h"
#include "monitor.h"
ecall_callback_t ecall_callback;

void set_ecall_callback(ecall_callback_t callback) {
	ecall_callback = callback;
}

void ecall(State* state, word* instruction) {
#ifdef RUN_TESTS //plain ecalls in test mode
	ecall_callback(state);
#else
#ifndef NO_TRACE_ECALLS
	if (state->privilege == PRIV_U) {
#define REG_A 10
		word nr = state->x[REG_A + 7];
		printf("ECALL (U) %s : a7=%u a0=%u a1=%x a2=%x a3=%x a4=%x a5=%x\n", syscall_name(nr), nr, \
			state->x[REG_A + 0], state->x[REG_A + 1], state->x[REG_A + 2], state->x[REG_A + 3], state->x[REG_A + 4], state->x[REG_A + 5]);
		//syscall specific output
		switch (nr) {
		case 56: //openat
		case 48: //fsaccessat
		case 291: //statx
		case 78: //readlinkat
			//a0 is string
		{
			word ptr = state->x[REG_A + 1];
			printf("filename: ");
			dump_string_virtual(state, ptr);
			printf("\n");
			//run_monitor(state);
		}
		break;
		case 422:
		case 98: //futex
			run_monitor(state);
		}
		getch();
	}
#endif
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