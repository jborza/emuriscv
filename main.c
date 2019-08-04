#include <stdio.h>
#include "cpu.h"
#include <memory.h>
#include "ecall.h"
#include "test.h"
#include "decode.h"

int last_exit_code = 0;
int bin_file_size = 0;
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

void test_beq_1() {
	int program[] = {
		0x00000463, //beq x0,x0,0x4
		0x00900093, //addi x1, x0, 0x9
		0x00000013, //nop / addi x0,x0,0
	};
	State state;
	clear_state(&state);
	memcpy(state.memory, program, sizeof(program));
	emulate_op(&state);
	print_registers(&state);
	emulate_op(&state);
	if (state.x[1] != 0x0) {
		printf("Assertion failed, x1 should be 0!");
		exit(1);
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

//uint32_t bextr(uint32_t src, uint32_t start, uint32_t len) {
//	return (src >> start) & ((1 << len) - 1);
//}
//
//int32_t shamt(word value) {
//	return bextr(value, 20, 6); 
//}
//
//uint32_t imm_sign(word value) {
//	return bextr(value, 31, 1);
//}
//
//int32_t get_b_imm(word value) {
//	return (bextr(value, 8, 4) << 1) + (bextr(value, 25, 6) << 5) + (bextr(value, 7, 1) << 11) + (imm_sign(value) << 12);
//}

void assert_shamt(word instruction, int expected_shamt) {
	int actual_shamt = shamt(instruction);
	if (actual_shamt != expected_shamt) {
		printf("Unexpected shamt value: %d, expected %d", actual_shamt, expected_shamt);
		exit(1);
		return;
	}
}

void test_shamt() {
	word instr = 0x00009f13; //slli x30, x1, 0
	assert_shamt(0x00009f13 /*slli x30, x1, 0*/, 0);
	assert_shamt(0x00109f13 /*slli x30, x1, 1*/, 1);
	assert_shamt(0x00709f13 /*slli x30, x1, 7*/, 7);
	assert_shamt(0x00e09f13 /*slli x30, x1, 14*/, 14);
	assert_shamt(0x01f09f13 /*slli x30, x1, 31*/, 31);
}

void assert_b_imm(word instruction, int expected_imm) {
	int actual = get_b_imm(instruction);
	if (actual != expected_imm) {
		printf("Unexpected B-imm value: %d, expected %d", actual, expected_imm);
		exit(1);
		return;
	}
}

void test_b_imm() {
	assert_b_imm(0xfe000ee3 /* beq x0, x0, 0xfffffffe */, 0xfffffffe * 2);
	assert_b_imm(0x00108a63 /* beq x1, x1, 0x0000000a */, 0x0000000a * 2);
	assert_b_imm(0x08108263 /* beq x1, x1, 0x00000042 */, 0x00000042 * 2);
	assert_b_imm(0x16108663 /* beq x1, x1, 0x000000b6 */, 0x000000b6 * 2);
	assert_b_imm(0x1a108e63 /* beq x1, x1, 0x000000de */, 0x000000de * 2);
	assert_b_imm(0x1e108e63 /* beq x1, x1, 0x000000fe */, 0x000000fe * 2);
	assert_b_imm(0x24108463 /* beq x1, x1, 0x00000124 */, 0x00000124 * 2);
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

void test_bin(char* name) {
	printf("Starting test for: %s\n", name);
	byte* buffer = read_bin(name, &bin_file_size);
	if (buffer == NULL) {
		return;
	}

	State state;
	clear_state(&state);
	memcpy(state.memory, buffer, bin_file_size);

	for (;;) {
		word* address = state.memory + state.pc;
		printf("0x%08x\t", *address);
		printf("pc: 0x%08x\tx1: 0x%08x\t", state.pc, state.x[1]);
		emulate_op(&state);
		//print_registers(&state);
		if (state.status == EXIT_TERMINATION) {
			check_test_exit_code();
			break;
		}
	}
	printf("Test %s passed\n", name);
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
	test_bin("test/beq_bne_loop.bin");
	test_shamt();
	test_b_imm();
	test_beq_1();
	test_bin("test/simple.bin");
	test_bin("test/beq.bin");
	test_bin("test/sltu.bin");
	test_bin("test/slli.bin");
	test_bin("test/addi.bin");
	test_bin("test/add.bin");
	//	test_bin("test/sub.bin");
	//	test_bin("test/slti.bin");

	printf("--------------------------\n");
	printf("ALL TESTS PASSED\n");
	printf("--------------------------\n");
}