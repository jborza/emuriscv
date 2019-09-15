#pragma once
#include "types.h"

typedef struct MemoryMap MemoryMap;
#define PHYS_MEM_RANGE_MAX 32

typedef struct MemoryRange {
	MemoryMap* map;
	uint32_t address;
	uint32_t size;
	uint8_t* phys_mem_ptr;
	void* opaque;
	void* read_func;
	void* write_func;
	int is_ram;
} MemoryRange;

typedef struct MemoryMap {
	int n_phys_mem_range;
	MemoryRange phys_mem_range[PHYS_MEM_RANGE_MAX];
} MemoryMap;

typedef void DeviceWriteFunc(void* opaque, uint32_t offset,
	uint32_t val, int size_log2);
typedef uint32_t DeviceReadFunc(void* opaque, uint32_t offset, int size_log2);

MemoryRange* register_ram_entry(MemoryMap* map, uint32_t base_addr, uint32_t size);
MemoryRange* cpu_register_ram(MemoryMap* map, uint32_t base_addr, uint32_t size);
MemoryRange* cpu_register_device(MemoryMap* s, uint64_t addr,
	uint64_t size, void* opaque,
	DeviceReadFunc* read_func, DeviceWriteFunc* write_func);
MemoryMap* phys_mem_map_init();