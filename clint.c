#include "clint.h"
#include "vm.h"

uint32_t clint_read(void* opaque, uint32_t offset, int size_log2)
{
	RiscVMachine* vm = opaque;
	uint32_t val;

	switch (offset) {
	case SIFIVE_TIME_LOW:
		val = rtc_get_time(vm);
		break;
	case SIFIVE_TIME_HIGH:
		val = rtc_get_time(vm) >> 32;
		break;
	case SIFIVE_TIMECMP_LOW: 
		val = vm->timecmp;
		break;
	case SIFIVE_TIMECMP_HIGH: 
		val = vm->timecmp >> 32;
		break;
	default:
		val = 0;
		break;
	}
	return val;
}

void clint_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2)
{
	RiscVMachine* vm = opaque;

	//assert(size_log2 == 2);
	switch (offset) {
	case SIFIVE_TIMECMP_LOW:
		vm->timecmp = (vm->timecmp & ~0xffffffff) | val;
		reset_mip(MIP_MTIP);
		//TODO reset MIP MIP_MTIP, see QEMU sifive_clint.c -> sifive_clint_write_timecmp()
		//riscv_cpu_reset_mip(m->cpu_state, MIP_MTIP);
		break;
	case SIFIVE_TIMECMP_HIGH:
		vm->timecmp = (vm->timecmp & 0xffffffff) | ((uint64_t)val << 32);
		reset_mip(MIP_MTIP);
		//TODO reset MIP MIP_MTIP
		//riscv_cpu_reset_mip(m->cpu_state, MIP_MTIP);
		break;
	default:
		break;
	}
}


static uint64_t rtc_get_time(RiscVMachine* vm)
{
	uint64_t val;
	//fake clock based on instructions emulated
	val = vm->cycles;
	return val;
}