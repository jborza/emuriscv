#include <stdio.h>
#include "cpu.h"
#include <memory.h>
#include "ecall.h"
#include "test.h"

int last_exit_code = 0;
const int SUCCESS = 42;
const int FAIL = 0;

void set_last_exit_code(int code) {
	last_exit_code = code;
}

void check_test_exit_code() {
	if (last_exit_code == FAIL) {
		printf("TEST FAILED!\n");
		exit(1);
	}
}

void clear_state(State* state) {
	//clear the registers
	for (int i = 0; i < REGISTERS; i++)
		state->x[i] = 0;
	state->pc = 0;
	state->status = RUNNING;
	//clear the memory
	const int memory_size = 1024 * 1024;
	state->memory = malloc(memory_size);
	memset(state->memory, 0, sizeof(byte) * memory_size);
	set_last_exit_code(0);
}

void print_registers(State* state) {
	for (int i = 0; i < 32; i++) {
		printf("x%d %08X\t", i, state->x[i]);
		if ((i + 1) % 4 == 0)
			printf("\n");
	}
}

void test_addi() {
	int program[] = {
		0x800000b7, //lui x1, 0x00080000
		0xfff08093, //addi x1, x1, 0xffffffff
		0x00108f13, //addi x30, x1, 0x001
	};
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (int i = 0; i < 3; i++)
	{
		emulate_op(&state);
		print_registers(&state);
	}
	if (state.x[30] != 0x80000000)
		printf("Assertion failed!");
}

void test_slli() {
	int program[] = {
		0x00033537,		//lui	    a0,0x33
		0xbfb50513,		//addi	a0,a0,-1029
		0x00e51513,		//slli	a0,a0,0xe
		0xabe50513		//addi	a0,a0,-1346
	};
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (int i = 0; i < 4; i++)
	{
		emulate_op(&state);
		print_registers(&state);
	}
	if (state.x[10] != 0xcafebabe)
		printf("Assertion failed!");
}

void test_add() {
	int program[] = {
		0x00100093, //li x1, 1
		0x00100113, //li x2, 1
		0x00208f33 //add x30, x1, x2
	};
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (int i = 0; i < 3; i++)
	{
		emulate_op(&state);
		print_registers(&state);
	}
	if (state.x[30] != 0x2)
		printf("Assertion failed!");
}

void test_add2() {
	int program[] = {
		 0x00100093, //	addi x1,x0,0x00000001     li x1, 0x00000001
		 0x00100113, // addi x2,x0,0x00000001     li x2, 0x00000001
		 0x00208f33, // add x30,x1,x2             add x30, x1, x2
		 0x00200e93, // addi x29,x0,0x00000002    li x29, 0x00000002
		 0x00300193, // addi x3,x0,0x00000003     li gp, 3
		 0x01df1863, // bne x30,x29,0x00000008    bne x30, x29, fail
		 0x02a00513, // addi x10,x0,0x0000002a    li a0,42
		 0x05d00893, // addi x17,x0,0x0000005d    li a7, 93
		 0x00000073, // ecall                     ecall
		 0x00000513, // addi x10,x0,0x00000000    li a0,0
		 0x05d00893, // addi x17,x0,0x0000005d    li a7, 93
		 0x00000073, // ecall                     ecall
	};
	//execute until ecall
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (;;) {
		word* address = state.memory + state.pc;
		printf("Instruction: 0x%08x\n", *address);
		emulate_op(&state);
		print_registers(&state);
		if (state.status == EXIT_TERMINATION) {
			check_test_exit_code();
			break;
		}
	}
	printf("Test passed\n");
}

void test_simple_bin() {
	FILE* file = fopen("test/simple.bin", "rb");
	if (!file) {
		printf("Couldn't load test bin file!");
		exit(1);
	}
	fseek(file, 0, SEEK_END);
	int bin_file_size = ftell(file);
	rewind(file);
	byte* buffer = malloc(bin_file_size);
	size_t read = fread(buffer, sizeof(byte), bin_file_size, file);
	fclose(file);
	

	State state;
	clear_state(&state);
	memcpy(state.memory, buffer, bin_file_size);

	for (;;) {
		word* address = state.memory + state.pc;
		printf("Instruction: 0x%08x\n", *address);
		emulate_op(&state);
		print_registers(&state);
		if (state.status == EXIT_TERMINATION) {
			check_test_exit_code();
			break;
		}
	}
	printf("Test passed\n");
}

void test_ecall() {
	int program[] = {
		0x02a00513, //li a0, 42
		0x05d00893, //li a7, 93
		0x00000073 //ecall
	};
	//execute until ecall
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (;;) {
		word* address = state.memory + state.pc;
		printf("Instruction: 0x%08x\n", *address);
		emulate_op(&state);
		print_registers(&state);
		if (state.status == EXIT_TERMINATION) {
			check_test_exit_code();
			break;
		}
	}
	printf("Test passed\n");
}

void test_memory() {
	//TODO assume the compact configuration - .text at 0x0000, .data at 0x2000
	//TODO generalize test_simple_bin
}

void test_ecall_callback(State* state) {
	//tests use a7 = EXIT (93)
	if (state->x[SYSCALL_REG] == EXIT) {
		set_last_exit_code(state->x[SYSCALL_ARG0]);
		state->status = EXIT_TERMINATION;
	}
}

int main(int argc, char* argv[]) {
	set_ecall_callback(&test_ecall_callback);
	test_simple_bin();
	test_ecall();
	test_add2();
	test_addi();
	test_slli();
	test_add();
}