#include "monitor.h"
#include <string.h>
#include <stdio.h>
#include "memory.h"
#include <stdlib.h>
#include "disassembler/disassembler.h"

static char* reg_name[32] = {
"zero", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
"s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
"a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
"s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

static char* privilege_name[4] = {
	"U", "S", "?", "M"
};

// monitor usage: 
// x Displays memory at the specified virtual address using the specified format. 
// xp Displays memory at the specified physical address using the specified format. 
// xp /3i $eip

#define UNKNOWN_REGISTER -1

int find_register(char* name) {
	char xname[4];
	for (int i = 0; i < REGISTERS; i++) {
		sprintf(xname, "x%d", i);
		if (strcmp(xname, name) == 0 || strcmp(reg_name[i], name) == 0)
			return i;
	}
	return UNKNOWN_REGISTER;
}

word get_register_value(State* state, char* name) {
	if (strcmp(name, "pc") == 0) {
		return state->pc;
	}
	int register_index = find_register(name);
	if (register_index == UNKNOWN_REGISTER)
	{
		printf("Unknown register: %s\n", name);
		return 0;
	}
	return state->x[register_index];
}

void dump_registers(State* state) {
	char xname[4];
	for (int i = 0; i < REGISTERS; i++) {
		sprintf(xname, "x%d", i);
		printf("%5s (%3s) : 0x%08x ", reg_name[i], xname, state->x[i]);
		if (i % 4 == 3)
			printf("\n");
	}
	printf("pc: %08x", state->pc);
	printf(" priv: %s", privilege_name[state->privilege]);
	printf(" cycles: %ld", state->instruction_counter);
	printf("\n");
}

int dump_data_as_string_check_terminated(word data);

void dump_value(word data) {
	printf("%08x | %ld | ", data, data);
	dump_data_as_string_check_terminated(data);
	printf("\n");
}

void dump_register(State* state, char* name) {
	if (strcmp(name, "pc") == 0) {
		printf("pc: ");
		dump_value(state->pc);
		return;
	}
	int register_index = find_register(name);
	if (register_index == UNKNOWN_REGISTER)
	{
		printf("Unknown register: %s\n", name);
		return;
	}
	printf("%5s (x%d): ", reg_name[register_index], register_index);
	dump_value(state->x[register_index]);
}

void dump_one_memory_value(word address, word data) {
	printf("0x%08x: ", address);
	dump_value(data);
}

void dump_memory_physical(State* state, word start_address, size_t count) {
	//data_format: 'x' for hex, 'd' for decimal, 'u' for unsigned decimal, 'o' for octal, 'c' for char and 'i' for (disassembled) processor instructions
	for (word address = start_address; address < start_address + count * WORD_SIZE; address += WORD_SIZE) {
		word data = read_word_physical(state, address);
		dump_one_memory_value(address, data);
	}
}

void dump_memory_virtual(State* state, word start_address, size_t count) {
	MemoryTarget memory_target;
	for (word address = start_address; address < start_address + count * WORD_SIZE; address += WORD_SIZE) {
		int read_status = get_memory_target(state, address, LOAD, &memory_target);
		if (read_status != TRANSLATE_OK)
			printf("Cannot translate virtual address %08x\n", address);
		word data = read_word(state, address);
		dump_one_memory_value(address, data);
	}
}

void disassemble_word(State* state, word start_address, size_t count) {
	MemoryTarget memory_target;
	for (word address = start_address; address < start_address + count * WORD_SIZE; address += WORD_SIZE) {
		int read_status = get_memory_target(state, address, LOAD, &memory_target);
		if (read_status != TRANSLATE_OK)
			printf("Cannot translate virtual address %08x\n", address);
		word data = read_word(state, address);
		printf("%8x:\t%08x\t\t", address, data);
		disassemble_op(&data, address);
	}
}

int dump_data_as_string_check_terminated(word data) {
	unsigned char bytes[5];
	bytes[4] = 0;
	bytes[3] = (data >> 24) & 0xFF;
	bytes[2] = (data >> 16) & 0xFF;
	bytes[1] = (data >> 8) & 0xFF;
	bytes[0] = data & 0xFF;
	printf("%s", bytes);
	return (bytes[0] == 0 || bytes[1] == 0 || bytes[2] == 0 || bytes[3] == 0);
}

void dump_string_virtual(State* state, word start_address) {
	MemoryTarget memory_target;
	//read until \0
	printf("0x%08x: ", start_address);

	for (word address = start_address; ; address += WORD_SIZE) {
		int read_status = get_memory_target(state, address, LOAD, &memory_target);
		if (read_status != TRANSLATE_OK) {
			printf("Cannot translate virtual address %08x\n", address);
			break;
		}
		word data = read_word(state, address);
		if (dump_data_as_string_check_terminated(data)) {
			printf("\n");
			break;
		}
	}
}

void write_memory_virtual(State* state, word address, word value) {
	MemoryTarget memory_target;
	int read_status = get_memory_target(state, address, LOAD, &memory_target);
	if (read_status != TRANSLATE_OK)
		printf("Cannot translate virtual address %08x\n", address);
	write_word(state, address, value);
	printf("ok.\n");
}

word get_address(char* token) {
	return strtoul(token, NULL, 16);
}

word get_value_hex(char* token) {
	return strtoul(token, NULL, 16);
}

word get_repeat(size_t token_count, char** tokens) {
	size_t repeat = 1;
	if (token_count > 2)
		repeat = atoi(tokens[2]);
	return repeat;
}

int run_monitor_loop(State* state) {
	printf("(emuriscv) ");
	char buffer[64];
	char* tokens[10];
	if (fgets(buffer, 64, stdin) == NULL)
		return -1;
	buffer[strcspn(buffer, "\n")] = 0;
	char* line = _strdup(buffer);
	size_t token_count = 0;
	for (token_count = 0; token_count < 10; token_count++) {
		char* arg = token_count == 0 ? line : NULL;
		tokens[token_count] = strtok(arg, " ");
		if (tokens[token_count] == NULL)
			break;
	}

	if (strcmp(tokens[0], "help") == 0 || strcmp(tokens[0], "?") == 0) {
		printf("help:\n");
		printf("regs - dump registers\n");
		printf("reg $reg - dump one register $reg (format: x7 or t0)\n");
		printf("x $addr [n] - dump n words from virtual address $addr\n");
		printf("xp $addr [n] - dump n words from physical address $addr\n");
		printf("xs $addr [n] - print a null terminated string from physical address $addr\n");
		printf("w $addr $val - write 1 words with value $val to from virtual address $addr\n");
		printf("p $reg [n] - dump n words from virtual address pointed to by $reg\n");
		printf("pp $reg [n] - dump n words from physical address pointed to by $reg\n");
		printf("d $addr [n] - disassembler n words from virtual address $addr\n");
		printf("dr $reg [n] - disassembler n words from virtual address pointed to by $reg\n");
		printf("step [n] - do n instructions\n");
		printf("q - quit\n");
	}
	else if (strcmp(tokens[0], "q") == 0) {
		return 1;
	}
	else if (strcmp(tokens[0], "regs") == 0) {
		dump_registers(state);
	}
	else if (strcmp(tokens[0], "reg") == 0) {
		if (token_count > 1)
			dump_register(state, tokens[1]);
	}
	else if (strcmp(tokens[0], "p") == 0) {
		if (token_count > 1) {
			word address = get_register_value(state, tokens[1]);
			size_t repeat = get_repeat(token_count, tokens);
			dump_memory_virtual(state, address, repeat);
		}
	}
	else if (strcmp(tokens[0], "pp") == 0) {
		if (token_count > 1) {
			word address = get_register_value(state, tokens[1]);
			size_t repeat = get_repeat(token_count, tokens);
			dump_memory_physical(state, address, repeat);
		}
	}
	else if (strcmp(tokens[0], "x") == 0) {
		if (token_count > 1) {
			word address = get_address(tokens[1]);
			size_t repeat = get_repeat(token_count, tokens);
			dump_memory_virtual(state, address, repeat);
		}
	}
	else if (strcmp(tokens[0], "xs") == 0) {
		if (token_count > 1) {
			word address = get_address(tokens[1]);
			dump_string_virtual(state, address);
		}
	}
	else if (strcmp(tokens[0], "xp") == 0) {
		word address = strtoul(tokens[1], NULL, 16);
		size_t repeat = 1;
		if (token_count > 2)
			repeat = atoi(tokens[2]);
		dump_memory_physical(state, address, repeat);
	}
	else if (strcmp(tokens[0], "w") == 0) {
		if (token_count > 2) {
			word address = get_address(tokens[1]);
			word value = get_value_hex(tokens[2]);
			write_memory_virtual(state, address, value);
		}
	}
	else if (strcmp(tokens[0], "d") == 0) {
		if (token_count > 1) {
			word address = get_address(tokens[1]);
			size_t repeat = get_repeat(token_count, tokens);
			disassemble_word(state, address, repeat);
		}
	}
	else if (strcmp(tokens[0], "dr") == 0) {
		if (token_count > 1) {
			word address = get_register_value(state, tokens[1]);
			size_t repeat = get_repeat(token_count, tokens);
			disassemble_word(state, address, repeat);
		}
	}
	else if (strcmp(tokens[0], "step") == 0) {
		if (token_count > 1) {
			size_t repeat = get_repeat(token_count, tokens);
			for (size_t i = 0; i < repeat; i++) {
				//TODO hack, should be probably somewhere else
				emulate_op(state);
				printf(".");
			}
			printf("\n");
		}
	}
	return 0;
}


void run_monitor(State* state) {
	for (;;)
	{
		if (run_monitor_loop(state) != 0)
			return;
	}
}