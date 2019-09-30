#pragma once
#include "types.h"
#include "vm.h"

int riscv_build_fdt(RiscVMachine* m, uint8_t* dst,
	uint64_t kernel_start, uint64_t kernel_size,
	const char* cmd_line);