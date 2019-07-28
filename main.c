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

int main(int argc, char* argv[]) {
	test_addi();
	int program[] =
	{
		0xcafec537,                //lui     a0,0xcafec
		0xabe50513                //addi    a0,a0,-1346 # 0xcafebabe
	};

	int program2[] = {
		0x00032537,//          	lui	    a0,0x32
		0xbfb50513,//          	addi	a0,a0,-1029
		0x00e51513,//          	slli	a0,a0,0xe
		0xabe50513//          	addi	a0,a0,-1346
	};

	int program3[] = { 0x0ff88893 };

	State state;
	clear_state(&state);

	//load program
	memcpy(state.memory, program3, sizeof(program));

	print_registers(&state);

	emulate_op(&state);
	print_registers(&state);
	emulate_op(&state);
	print_registers(&state);

}