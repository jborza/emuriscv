#include "memory.h"
#include <stdio.h>

void* mallocz(size_t size)
{
	void* ptr;
	ptr = malloc(size);
	if (!ptr)
		return NULL;
	memset(ptr, 0, size);
	return ptr;
}

uint8_t* get_physical_address(State* state, uint32_t address) {
	uint8_t* ptr;
	MemoryRange* pr;

	uint32_t physical_address = address;

	pr = get_phys_mem_range(state->memory_map, physical_address);
	if (!pr) {
		printf("get_physical_address: invalid physical address 0x%08x\n", physical_address);
		return 0;
	}
	//if it's not ram, device IO
	ptr = pr->phys_mem_ptr + (uint32_t)(physical_address - pr->address);
	return ptr;
}

word * fetch_next_word(State * state) {
	uint8_t* address = get_physical_address(state, state->pc);
	state->pc += 4;
	return address;
}

void write_common_ram(State * state, uint8_t * target, word value, int size_log2) {
	switch (size_log2) {
	case SIZE_WORD:
		*(target + 3) = (value & 0xff000000) >> 24;
		*(target + 2) = (value & 0xff0000) >> 16;
	case SIZE_HALF:
		*(target + 1) = (value & 0xff00) >> 8;
	case SIZE_BYTE:
		*target = value & 0xff;
		break;
	default:
		fprintf(stderr, "write_common_ram unsupported size:%d", size_log2);
		break;
	}
}

typedef struct MemoryTarget {
	MemoryRange* range;
	uint8_t* ptr;
} MemoryTarget;

MemoryTarget get_memory_target(State * state, word address) {
	MemoryTarget target;
	uint8_t* ptr;
	uint32_t target_address = address;
	target.range = get_phys_mem_range(state->memory_map, target_address);

	if (!target.range) {
		printf("get_physical_address: invalid physical address 0x%08x\n", target_address);
		exit(1);
		return target;
	}
	target.ptr = target.range->phys_mem_ptr + (uint32_t)(target_address - target.range->address);
	return target;
}

void write_common(State * state, word address, word value, int size_log2) {
	MemoryTarget target = get_memory_target(state, address);
	if (target.range->is_ram) {
		write_common_ram(state, target.ptr, value, size_log2);
	}
	else {
		//device I/O
 		target.range->write_func(target.range->opaque, target.ptr, value, size_log2);
	}
}

void write_word(State * state, word address, word value) {
	write_common(state, address, value, SIZE_WORD);
}

void write_halfword(State * state, word address, halfword value) {
	write_common(state, address, value, SIZE_HALF);
}

void write_byte(State * state, word address, byte value) {
	write_common(state, address, value, SIZE_BYTE);
}

// READ 

word read_common_ram(State * state, uint8_t * target, int size_log2) {
	word value;
	switch (size_log2) {
	case SIZE_WORD:
		value = *target
			| *(target + 1) << 8
			| *(target + 2) << 16
			| *(target + 3) << 24;
		break;
	case SIZE_HALF:
		value = *target | *(target + 1) << 8;
		break;
	case SIZE_BYTE:
		value = *target;
		break;
	default:
		fprintf(stderr, "read_common_ram unsupported size:%d", size_log2);
		exit(1);
		return 0;
	}
	return value;
}

word read_common(State * state, word address, int size_log2) {
	MemoryTarget target = get_memory_target(state, address);
	if (target.range->is_ram) {
		return read_common_ram(state, target.ptr, size_log2);
	}
	else {
		//device I/O
		return target.range->read_func(target.range->opaque, target.ptr, size_log2);
	}
}

word read_word(State * state, word address) {
	return read_common(state, address, SIZE_WORD);
}

word read_halfword_signed(State * state, word address) {
	word value = read_common(state, address, SIZE_HALF);
	//sign extend 16-bit value
	if ((value & 0x8000) == 0x8000)
		return value | 0xffff0000;
	else
		return value;
}

word read_halfword_unsigned(State * state, word address) {
	word value = read_common(state, address, SIZE_HALF);
	return value;
}

word read_byte_signed(State * state, word address) {
	word value = read_common(state, address, SIZE_BYTE);
	//sign extend 8-bit value
	if ((value & 0x80) == 0x80)
		return value | 0xffffff00;
	else
		return value;
}

word read_byte_unsigned(State * state, word address) {
	word value = read_common(state, address, SIZE_BYTE);
	return value;
}