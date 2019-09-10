#include <stdio.h>
#include "cpu.h"
#include <memory.h>
#include "test.h"
#include "decode.h"
#include "memory.h"
#include "vm.h"

const int ram_size = 16 * 1024 * 1024;
State state;

void clear_state_linux(State* state) {
	//TODO would be better to initialize with mallocz
	//clear the registers
	for (int i = 0; i < REGISTERS; i++)
		state->x[i] = 0;
	for (int i = 0; i < CSR_REGISTERS; i++)
		state->csr[i] = 0;
	state->mhartid = 0;
	state->pc = 0;
	state->status = RUNNING;
	state->instruction_counter = 0;
	//clear the memory
	//const int memory_size = 1024 * 1024;
	//state->memory = malloc(ram_size);
	//memset(state->memory, 0, sizeof(byte) * ram_size);
}

byte* read_bin(char* name, int* bin_file_size) {
	FILE* file = fopen(name, "rb");
	if (!file) {
		printf("Couldn't load test bin file '%s'!", name);
		exit(1);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	*bin_file_size = ftell(file);

	rewind(file);
	byte* buffer = malloc(*bin_file_size);
	size_t read = fread(buffer, sizeof(byte), *bin_file_size, file);
	fclose(file);
	return buffer;
}

void linux_ecall_callback(State * state) {
	//TODO implement
	printf("!! TODO !! ecall_callback\n");
}

#define vm_error(...) printf(__VA_ARGS__)

MemoryRange* register_ram_entry(MemoryMap * map, uint32_t base_addr, uint32_t size) {
	MemoryRange* pr;
	if (map->n_phys_mem_range + 1 > PHYS_MEM_RANGE_MAX)
	{
		fprintf(stderr, "Maximum amount of memory ranges exceeded\n");
		exit(1);
	}
	pr = &(map->phys_mem_range[map->n_phys_mem_range++]); 
	pr->map = map;
	pr->size = size;
	pr->address = base_addr;
	return pr;
}

MemoryRange* cpu_register_ram(MemoryMap * map, uint32_t base_addr, uint32_t size) {
	//register the range
	MemoryRange* range = register_ram_entry(map, base_addr, size);
	//allocate new memory for the range
	range->phys_mem = mallocz(size);
	if (!range->phys_mem) {
		fprintf(stderr, "Could not allocate VM memory\n");
		exit(1);
	}
	return range;
}

MemoryMap* phys_mem_map_init() {
	MemoryMap* s;
	s = mallocz(sizeof(*s));
	return s;
}

RiscVMachine* initialize_riscv_machine() {
	//TODO clean up
	RiscVMachine* vm;
	//riscv32 only
	int max_xlen = 32;

	vm = mallocz(sizeof(*vm));
	vm->ram_size = ram_size;
	vm->max_xlen = max_xlen;
	vm->mem_map = phys_mem_map_init();

	cpu_register_ram(vm->mem_map, RAM_BASE_ADDR, ram_size);
	cpu_register_ram(vm->mem_map, 0x00000000, LOW_RAM_SIZE);

	return vm;
}

MemoryRange* get_phys_mem_range(MemoryMap* map, uint32_t paddr) {
	MemoryRange* range;
	for (int i = 0; i < map->n_phys_mem_range; i++)
	{
		range = &(map->phys_mem_range[i]);
		if (paddr >= range->address && paddr < range->address + range->size)
			return range;
	}
	return NULL;
}

uint8_t* phys_mem_get_ram_ptr(MemoryMap* map, uint32_t paddr/*, BOOL is_rw*/) {
	MemoryRange* pr = get_phys_mem_range(map, paddr);
	uintptr_t offset;
	if (!pr)
		return NULL;
	offset = paddr - pr->address;
	return pr->phys_mem + (uintptr_t)offset;
}

static uint8_t* get_ram_ptr(RiscVMachine* s, uint32_t paddr/*, BOOL is_rw*/)
{
	return phys_mem_get_ram_ptr(s->mem_map, paddr);
}

void load_bios_and_kernel(RiscVMachine *vm) {
	int buf_len;
	uint8_t* buf = read_bin("linux/bbl32.bin", &buf_len);

	if (buf_len > vm->ram_size) {
		vm_error("BIOS too big\n");
		exit(1);
	}

	uint8_t* ram_ptr = get_ram_ptr(vm, RAM_BASE_ADDR);
	memcpy(ram_ptr, buf, buf_len);

	//TODO load kernel

	//TODO load flattened device tree
	ram_ptr = get_ram_ptr(vm, 0);
	uint32_t fdt_addr = 0x1000 + 8 * 8;

	uint32_t jump_addr = 0x80000000;

	//set up BBL for loading
	//boot from 0x1000, then jump to 0x80000000
	uint32_t* q = (uint32_t*)(ram_ptr + 0x1000);
	q[0] = 0x297 + jump_addr - 0x1000; /* auipc t0, jump_addr */
	q[1] = 0x597; /* auipc a1, dtb */
	q[2] = 0x58593 + ((fdt_addr - 4) << 20); /* addi a1, a1, dtb */
	q[3] = 0xf1402573; /* csrr a0, mhartid */
	q[4] = 0x00028067; /* jalr zero, t0, jump_addr */
}

void run_linux() {
	clear_state_linux(&state);
	//initialize machine
	RiscVMachine *vm = initialize_riscv_machine();
	//TODO refactor state and RiscVMachine together
	state.memory_map = vm->mem_map;
	//set up syscall callback
	set_ecall_callback(&linux_ecall_callback);
	//load bios
	load_bios_and_kernel(vm);

	//the initial loader address
	state.pc = 0x1000;

	for (;;) {
		word* address = get_physical_address(&state, state.pc);
		printf("%08x:\t%08x\t\t", state.pc, *address);
		emulate_op(&state);
	}
}

int main(int argc, char* argv[]) {
	run_linux();
}