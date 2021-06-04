#include <stdio.h>
#include "config.h"
#include "cpu.h"
#include "cpu_ecall.h"
#include <memory.h>
#include "test.h"
#include "decode.h"
#include "memory.h"
#include "vm.h"
#include "fdt.h"
#include "debug_symbols.h"
#include "support_io.h"
#include <time.h>
#include "sbi.h"
#include "memory_utils.h"
#include <stdlib.h>
#include <crtdbg.h>
#include "monitor.h"
#include "uart.h"
#include "csr.h"
#include "exit_codes.h"

const int ram_size = VM_MEMORY_SIZE;
State* state;

#define SYSCALL_REG 17
#define EXIT 93
#define SYSCALL_ARG0 10

int print_verbose = 0;

const uint32_t BOOTLOADER_ADDRESS = 0x1000;
const uint32_t kernel_relocated_base = 0xc0000000;

//    /* HTIF */
static uint32_t htif_read(void* opaque, uint32_t offset,
	int size_log2);
static void htif_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2);

static uint32_t clint_read(void* opaque, uint32_t offset, int size_log2);
static void clint_write(void* opaque, uint32_t offset, uint32_t val, int size_log2);

static uint32_t uart_read(void* opaque, uint32_t offset, int size_log2);
static void uart_write(void* opaque, uint32_t offset, uint32_t val, int size_log2);

State* initialize_state_linux() {
	State* state = mallocz(sizeof(*state));
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
	state->privilege = PRIV_M;
	//HACK hardcode MISA
	//IMAS -> bits 0, 8, 12, 18, XLEN32 (bit 30)
	state->csr[CSR_MISA] = 1 << 0 | 1 << 8 | 1 << 12 | 1 << 18 | 1 << 30;
	state->csr[CSR_MHARTID] = state->mhartid;
	return state;
}

void linux_ecall_callback(State* state) {
	//TODO switch instead of if
#ifdef ENABLE_CONSOLE
	if (state->x[SYSCALL_REG] == SBI_CONSOLE_PUTCHAR) {
		char c = (char)state->x[SBI_ARG0_REG];
		fprintf(stdout, "%c", c);
		//TODO respond with ACK?
		state->x[SBI_RETURN_REG] = SBI_SUCCESS;
	}
	else if (state->x[SYSCALL_REG] == SBI_CONSOLE_GETCHAR) {
		//set some return value in the register a0
		state->x[SBI_RETURN_REG] = -1;
	}
#endif
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

	cpu_register_device(vm->mem_map, HTIF_BASE_ADDR, HTIF_SIZE,
		vm, htif_read, htif_write);

	cpu_register_device(vm->mem_map, CLINT_BASE_ADDR, CLINT_SIZE,
		vm, clint_read, clint_write);

	cpu_register_device(vm->mem_map, UART_BASE_ADDR, UART_SIZE,
		vm, uart_read, uart_write);

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
	return pr->phys_mem_ptr + (uintptr_t)offset;
}

static uint8_t* get_ram_ptr(RiscVMachine* s, uint32_t paddr/*, BOOL is_rw*/)
{
	return phys_mem_get_ram_ptr(s->mem_map, paddr);
}

void write_bootloader(uint8_t* ram_ptr, word fdt_addr) {
	word* q = (word*)(ram_ptr + BOOTLOADER_ADDRESS);
	q[0] = 0x297 + RAM_BASE_ADDR - 0x1000; /* auipc t0, jump_addr */
	q[1] = 0x597; /* auipc a1, dtb */
	q[2] = 0x58593 + ((fdt_addr - 4) << 20); /* addi a1, a1, dtb */
	q[3] = 0xf1402573; /* csrr a0, mhartid */
	q[4] = 0x00028067; /* jalr zero, t0, jump_addr */
}

void load_bios_and_kernel(RiscVMachine* vm) {
	int buf_len, kernel_buf_len;
	word kernel_align, kernel_base;
	uint8_t* buf = read_bin(BOOTLOADER_BINARY, &buf_len);

	if (buf_len > vm->ram_size) {
		fprintf(stderr, "bootloader too big\n");
		exit(EXIT_BOOTLOADER_TOO_BIG);
	}

	uint8_t* ram_ptr = get_ram_ptr(vm, RAM_BASE_ADDR);
	memcpy(ram_ptr, buf, buf_len);

	//load kernel
	uint8_t* kernel_buf = read_bin(LINUX_BINARY, &kernel_buf_len);
	if (kernel_buf_len > 0) {
		/* copy the kernel if present */
		kernel_align = 4 << 20; /* 4 MB page align */
		kernel_base = (buf_len + kernel_align - 1) & ~(kernel_align - 1);
		memcpy(ram_ptr + kernel_base, kernel_buf, kernel_buf_len);
	}
	else {
		kernel_base = 0;
	}

	//load flattened device tree
	ram_ptr = get_ram_ptr(vm, 0);
	word fdt_addr = BOOTLOADER_ADDRESS + 8 * 8;

	char* cmd_line = LINUX_CMDLINE;

#ifdef BUILD_REAL_FDT
	riscv_build_fdt(vm, ram_ptr + fdt_addr,
		(uint64_t)RAM_BASE_ADDR + kernel_base,
		kernel_buf_len, cmd_line);
#else
	riscv_load_fdt("linux/spike_dts.bin", ram_ptr + fdt_addr);
#endif
	uint32_t jump_addr = RAM_BASE_ADDR;
	write_bootloader(ram_ptr, fdt_addr);
}

symbol* add_symbol(symbol* tail, word offset, char* name) {
	symbol* current = mallocz(sizeof(struct symbol));
	current->offset = offset;
	current->name = name;
	if (tail != NULL)
		tail->next = current;
	return current;
}

symbol* get_symbol(symbol* symbol_head, word address) {
	symbol* current = symbol_head;
	symbol* candidate = current;
	while (current->offset <= address && current->next != NULL) {
		candidate = current;
		current = current->next;
	}
	return candidate;
}

void console_write(const uint8_t* buf, int len) {
#ifdef ENABLE_CONSOLE
	fprintf(stderr, "%c", *buf);
#endif
#ifdef OUTPUT_CONSOLE_TO_FILE

#endif
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
		exit(EXIT_POWEROFF);
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

static uint64_t rtc_get_time(RiscVMachine* vm)
{
	uint64_t val;
	//fake clock based on instructions emulated
	val = vm->cycles;
	return val;
}

int32_t uart_reg[7];


static uint32_t uart_read(void* opaque, uint32_t offset, int size_log2)
{
	uint32_t val;
	int offset_words = offset >> 2;
	if (offset_words == UART_REG_TXFIFO) {
		return 0;
	}
	//else if (offset_words == UART_REG_RXFIFO) {
		//return (int)'!';

	//}
	return uart_reg[offset_words];
}

static void uart_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2)
{
	int offset_words = offset >> 2;
	uart_reg[offset_words] = val;
	if (offset_words == UART_REG_TXFIFO) {
		//if(val!=0 && val != 0x40 && val != 0x5e)
		if (val != 0)
			fputc(val, stderr);
	}
}

static uint32_t clint_read(void* opaque, uint32_t offset, int size_log2)
{
	RiscVMachine* vm = opaque;
	uint32_t val;

	//assert(size_log2 == 2);
	switch (offset) {
	case 0xbff8:
		val = rtc_get_time(vm);
		break;
	case 0xbffc:
		val = rtc_get_time(vm) >> 32;
		break;
	case 0x4000:
		val = vm->timecmp;
		break;
	case 0x4004:
		val = vm->timecmp >> 32;
		break;
	default:
		val = 0;
		break;
	}
	return val;
}

//#define MIP_MTIP (1 << 7)

static void clint_write(void* opaque, uint32_t offset, uint32_t val,
	int size_log2)
{
	RiscVMachine* vm = opaque;

	//assert(size_log2 == 2);
	switch (offset) {
	case 0x4000:
		vm->timecmp = (vm->timecmp & ~0xffffffff) | val;
		//riscv_cpu_reset_mip(m->cpu_state, MIP_MTIP);
		break;
	case 0x4004:
		vm->timecmp = (vm->timecmp & 0xffffffff) | ((uint64_t)val << 32);
		//riscv_cpu_reset_mip(m->cpu_state, MIP_MTIP);
		break;
	default:
		break;
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
	state = initialize_state_linux();

	//initialize machine
	RiscVMachine* vm = initialize_riscv_machine();

	//TODO refactor state and RiscVMachine together
	state->memory_map = vm->mem_map;
	//set up syscall callback
	set_ecall_callback(&linux_ecall_callback);
	//load bios
	load_bios_and_kernel(vm);

	//the initial loader address
	state->pc = BOOTLOADER_ADDRESS;
	symbol* symbol = NULL;
#ifdef PRINT_OPCODES_ALWAYS
	print_verbose = 1;
#endif
	for (;;) {
#ifdef RUN_LINUX_VERBOSE
		if (print_verbose == 1) {
			//word* address = get_physical_address(state, state->pc);
			//symbol = get_symbol(symbol_list, state->pc);
			MemoryTarget next_op_target;
			int read_status = get_memory_target(state, state->pc, FETCH, &next_op_target);
			word* instruction = next_op_target.ptr;
			symbol = get_symbol(symbol_list, state->pc);
			printf("%08x:  %08X", state->pc, *instruction);
			printf(" %s  \n", symbol->name);
		}
#endif
		
#ifdef SAMPLE_TRACING
		if (state->instruction_counter % sampling_period == 0) {
			symbol = get_symbol(symbol_list, state->pc);
			printf("%08x PRV:%s @%08ld", state->pc, state->privilege == PRIV_U ? "U" : state->privilege == PRIV_S ? "S" : "U", state->instruction_counter);
			printf(" %s  \n", symbol->name);
		}

#endif
		emulate_op(state);
		vm->cycles = state->instruction_counter;
	}
}

#ifdef RUN_LINUX
int main(int argc, char* argv[]) {
	_CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_WNDW);
	_CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
	run_linux();
}
#endif