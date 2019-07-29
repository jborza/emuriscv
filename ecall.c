#include "ecall.h"
ecall_callback_t ecall_callback;

void set_ecall_callback(ecall_callback_t callback) {
	ecall_callback = callback;
}