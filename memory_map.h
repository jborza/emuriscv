#pragma once
#include "types.h"

typedef struct MemoryMap MemoryMap;
#define PHYS_MEM_RANGE_MAX 32

typedef struct MemoryRange {
	MemoryMap* map;
	uint32_t address;
	uint32_t size;
	uint8_t* phys_mem;
	void* opaque;
	void* read_func;
	void* write_func;
	int is_ram;
} MemoryRange;

typedef struct MemoryMap {
	int n_phys_mem_range;
	MemoryRange phys_mem_range[PHYS_MEM_RANGE_MAX];
} MemoryMap;