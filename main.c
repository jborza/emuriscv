#include <stdio.h>
#include "cpu.h"
#include <memory.h>

void clear_state(State* state) {
	//clear the registers
	for (int i = 0; i < REGISTERS; i++)
		state->x[i] = 0;
	state->pc = 0;
	//clear the memory
	const int memory_size = 1024 * 1024;
	state->memory = malloc(memory_size);
	memset(state->memory, 0, sizeof(byte) * memory_size);
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
		0x00100093,
		0x00100113,
		0x00208f33,
		0x00200e93,
		0x00300193,
		0x01df1863,
		0x02a00513,
		0x05d00893,
		0x00000073,
		0x00000513,
		0x05d00893,
		0x00000073
	};
	//execute until ecall
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (;;) {
		printf("%8X\n", state.memory[state.pc]);
		emulate_op(&state);
		print_registers(&state);
	}
}

void test_ecall() {
	int program[] = {
		0x00000513, //li a0, 0
		0x05d00893, //li a7, 93
		0x00000073 //ecall
	};
	//execute until ecall
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	for (;;) {
		printf("%8X\n", state.memory[state.pc]);
		emulate_op(&state);
		print_registers(&state);
	}
}

ecall_callback_t test_ecall_callback(State* state) {
	//tests use a7 = 93
	if (state->x[17] == 93) {
		const int SUCCESS = 42;
		const int FAIL = 0;
		//compare a0 to 42 (success) or 0 (fail)
		if (state->x[10] == FAIL) {
			printf("Test failure!");
			exit(1);
		}
	}
}

int main(int argc, char* argv[]) {
	set_ecall_callback(&test_ecall_callback);
	test_ecall();
	test_add2();
	test_addi();
	test_slli();
	test_add();
}