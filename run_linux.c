#include <stdio.h>
#include "cpu.h"
#include <memory.h>
#include "test.h"
#include "decode.h"
#include "memory.h"
#include "vm.h"
#include "fdt.h"
#include "debug_symbols.h"

const int ram_size = 16 * 1024 * 1024;
State state;

#define SYSCALL_REG 17
#define EXIT 93
#define SYSCALL_ARG0 10
#define CONSOLE_PUTCHAR 1
#define CONSOLE_PUTINT16 2
#define CONSOLE_PUTINT32 3
#define CONSOLE_PUTSTRING 4

//#define BUILD_REAL_FDT

//    /* HTIF */
static uint32_t htif_read(void* opaque, uint32_t offset,
	int size_log2);
static void htif_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2);

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
	if (state->x[SYSCALL_REG] == CONSOLE_PUTCHAR) {
		char c = (char)state->x[SYSCALL_ARG0];
		fprintf(stderr, "%c", c);
	}
	else if (state->x[SYSCALL_REG] == CONSOLE_PUTINT16) {
		int value = state->x[SYSCALL_ARG0];
		fprintf(stderr, "%d", value);
	}
	else if (state->x[SYSCALL_REG] == CONSOLE_PUTINT32) {
		int value = state->x[SYSCALL_ARG0];
		fprintf(stderr, "%d", value);
	}
	else if (state->x[SYSCALL_REG] == CONSOLE_PUTSTRING) {
	int value = state->x[SYSCALL_ARG0];
		//note: translate address
		word* address = get_physical_address(state, value);
		fprintf(stderr, "%s", address);
	}
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
	pr->is_ram = 1;
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

typedef void DeviceWriteFunc(void* opaque, uint32_t offset,
	uint32_t val, int size_log2);
typedef uint32_t DeviceReadFunc(void* opaque, uint32_t offset, int size_log2);

//	cpu_register_device(vm->mem_map, addr:HTIF_BASE_ADDR, size: 16, opaque: vm, read: htif_read, write: htif_write, flags: DEVIO_SIZE32);

MemoryRange* cpu_register_device(MemoryMap* s, uint64_t addr,
	uint64_t size, void* opaque,
	DeviceReadFunc* read_func, DeviceWriteFunc* write_func,
	int devio_flags)
{
	MemoryRange* pr;
	pr = &s->phys_mem_range[s->n_phys_mem_range++];
	pr->map = s;
	pr->address = addr;
	//pr->org_size = size;
	//if (devio_flags & DEVIO_DISABLED)
	//	pr->size = 0;
	//else
	pr->size = size;// pr->org_size;
	pr->is_ram = 0;
	pr->opaque = opaque;
	pr->read_func = read_func;
	pr->write_func = write_func;
	//pr->devio_flags = devio_flags;
	return pr;
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

#define DEVIO_SIZE32 4


	cpu_register_device(vm->mem_map, HTIF_BASE_ADDR, 16,
		vm, htif_read, htif_write, DEVIO_SIZE32);

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
	uint32_t kernel_base = 0;
	uint32_t kernel_buf_len = 0;

	char* cmd_line = "console=htifcon0";

#ifdef BUILD_REAL_FDT
	riscv_build_fdt(vm, ram_ptr + fdt_addr,
		RAM_BASE_ADDR + kernel_base,
		kernel_buf_len, cmd_line);
#else
	riscv_load_fdt("linux/spike_dts.bin", ram_ptr+fdt_addr);
#endif
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


symbol* add_symbol(symbol *tail, int offset, char* name) {
	symbol* current = mallocz(sizeof(symbol));
	current->offset = offset;
	current->name = name;
	if (tail != NULL)
		tail->next = current;
	return current;
}


symbol* get_symbol(symbol *symbol_head, word address) {
	symbol* current = symbol_head;
	symbol* candidate = current;
	while (current->offset <= address && current->next != NULL) {
		candidate = current;
		current = current->next;
	}
	return candidate;
}

void console_write(const uint8_t* buf, int len) {
	fprintf(stderr, "%s", buf);
}

static uint32_t htif_read(void* opaque, uint32_t offset,
	int size_log2)
{
	RiscVMachine* s = opaque;
	uint32_t val;

	switch (offset) {
	case 0:
		val = s->htif_tohost;
		break;
	case 4:
		val = s->htif_tohost >> 32;
		break;
	case 8:
		val = s->htif_fromhost;
		break;
	case 12:
		val = s->htif_fromhost >> 32;
		break;
	default:
		val = 0;
		break;
	}
	return val;
}

static void htif_handle_cmd(RiscVMachine* s)
{
	uint32_t device, cmd;

	device = s->htif_tohost >> 56;
	cmd = (s->htif_tohost >> 48) & 0xff;
	if (s->htif_tohost == 1) {
		/* shuthost */
		printf("\nPower off.\n");
		exit(0);
	}
	else if (device == 1 && cmd == 1) {
		uint8_t buf[1];
		buf[0] = s->htif_tohost & 0xff;
		console_write(buf, 1);
		s->htif_tohost = 0;
		s->htif_fromhost = ((uint64_t)device << 56) | ((uint64_t)cmd << 48);
	}
	else if (device == 1 && cmd == 0) {
		/* request keyboard interrupt */
		s->htif_tohost = 0;
	}
	else {
		printf("HTIF: unsupported tohost=0x%016x\n", s->htif_tohost);
	}
}

static void htif_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2)
{
	RiscVMachine* s = opaque;

	switch (offset) {
	case 0:
		s->htif_tohost = (s->htif_tohost & ~0xffffffff) | val;
		break;
	case 4:
		s->htif_tohost = (s->htif_tohost & 0xffffffff) | ((uint64_t)val << 32);
		htif_handle_cmd(s);
		break;
	case 8:
		s->htif_fromhost = (s->htif_fromhost & ~0xffffffff) | val;
		break;
	case 12:
		s->htif_fromhost = (s->htif_fromhost & 0xffffffff) |
			(uint64_t)val << 32;
		break;
	default:
		break;
	}
}

void run_linux() {
	initialize_symbols();
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
	symbol* symbol = NULL;
	int do_output = 0;
	for (;;) {
		word* address = get_physical_address(&state, state.pc);
		symbol = get_symbol(symbol_list, state.pc);
		if (do_output) {
			printf("%08x:  %08x  ", state.pc, *address);
			printf("%s  ", symbol->name);
		}
		emulate_op(&state);
	}
}

int main(int argc, char* argv[]) {
	run_linux();
}