#pragma once
#include "types.h"
#include "vm.h"
//sifive clint

#define SIFIVE_TIMECMP_BASE 0x4000
#define SIFIVE_TIMECMP_LOW SIFIVE_TIMECMP_BASE
#define SIFIVE_TIMECMP_HIGH SIFIVE_TIMECMP_LOW+4

#define SIFIVE_TIME_BASE 0xBFF8
#define SIFIVE_TIME_LOW SIFIVE_TIME_BASE
#define SIFIVE_TIME_HIGH SIFIVE_TIME_LOW+4

#define MIP_MTIP (1 << 7)

/*static */uint32_t clint_read(void* opaque, uint32_t offset, int size_log2);
void clint_write(void* opaque, uint32_t offset, uint32_t val, int size_log2);
static uint64_t rtc_get_time(RiscVMachine* vm);