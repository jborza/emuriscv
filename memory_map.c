#include "memory_map.h"
#include <stdio.h>
#include "exit_codes.h"

MemoryRange* cpu_register_device(MemoryMap* s, uint64_t addr,
	uint64_t size, void* opaque,
	DeviceReadFunc* read_func, DeviceWriteFunc* write_func)
{
	MemoryRange* pr;
	pr = &s->phys_mem_range[s->n_phys_mem_range++];
	pr->map = s;
	pr->address = addr;
	//pr->org_size = size;
	pr->size = size;// pr->org_size;
	pr->is_ram = 0;
	pr->opaque = opaque;
	pr->read_func = read_func;
	pr->write_func = write_func;
	return pr;
}

MemoryMap* phys_mem_map_init() {
	MemoryMap* s;
	s = mallocz(sizeof(*s));
	return s;
}

MemoryRange* register_ram_entry(MemoryMap* map, uint32_t base_addr, uint32_t size) {
	MemoryRange* pr;
	if (map->n_phys_mem_range + 1 > PHYS_MEM_RANGE_MAX)
	{
		fprintf(stderr, "Maximum amount of memory ranges exceeded\n");
		exit(EXIT_TOO_MANY_MEMORY_RANGES);
	}
	pr = &(map->phys_mem_range[map->n_phys_mem_range++]);
	pr->map = map;
	pr->size = size;
	pr->address = base_addr;
	pr->is_ram = 1;
	return pr;
}

MemoryRange* cpu_register_ram(MemoryMap* map, uint32_t base_addr, uint32_t size) {
	//register the range
	MemoryRange* range = register_ram_entry(map, base_addr, size);
	//allocate new memory for the range
	range->phys_mem_ptr = mallocz(size);
	if (!range->phys_mem_ptr) {
		fprintf(stderr, "Could not allocate VM memory\n");
		exit(EXIT_CANNOT_ALLOCATE_VM_MEMORY);
	}
	return range;
}