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
		if ((i+1) % 4 == 0)
			printf("\n");
	}
}

int main(int argc, char* argv[]) {
	int program[] =
	{
		0xcafec537,                //lui     a0,0xcafec
		0xabe50513                //addi    a0,a0,-1346 # 0xcafebabe
	};

	State state;
	clear_state(&state);

	//load program
	memcpy(state.memory, program, sizeof(program));

	print_registers(&state);

	emulate_op(&state);
	print_registers(&state);
	emulate_op(&state);
	print_registers(&state);

}