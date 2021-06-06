#include "loadsave.h"
#include "memory_map.h"
#include <stdio.h>

void save(State* state, char* filename) {
	FILE* f = fopen(filename, "wb");
	//order of saving: state, memory ranges
	fwrite(state, sizeof(State), 1, f);
	//memory ranges - only physical
	for (int i = 0; i < state->memory_map->n_phys_mem_range; i++) {
		MemoryRange range = state->memory_map->phys_mem_range[i];
		if (!range.is_ram)
			continue;
		fwrite(&range.address, 1, sizeof(range.address), f);
		fwrite(&range.size, 1, sizeof(range.size), f);
		fwrite(range.phys_mem_ptr, sizeof(byte), range.size, f);
	}
	fclose(f);
}

void load(State* state, char* filename) {
	FILE* f = fopen(filename, "rb");
	//order of loading: state, memory ranges
	State oldState = *state;
	fread(state, sizeof(State), 1, f);
	//restore memory map from our old copy
	state->memory_map = oldState.memory_map;
	//memory ranges - only physical
	for (int i = 0; i < state->memory_map->n_phys_mem_range; i++) {
		MemoryRange range = state->memory_map->phys_mem_range[i];
		if (!range.is_ram)
			continue;
		fread(&range.address, 1, sizeof(range.address), f);
		fread(&range.size, 1, sizeof(range.size), f);
		fread(range.phys_mem_ptr, sizeof(byte), range.size, f);
	}
	fclose(f);
}