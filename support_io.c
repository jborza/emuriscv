#include "support_io.h"
#include <stdio.h>
#include "exit_codes.h"

byte* read_bin(char* name, int* bin_file_size) {
	FILE* file = fopen(name, "rb");
	if (!file) {
		printf("Couldn't load test bin file '%s'!", name);
		exit(EXIT_CANNOT_LOAD_TEST_BINARY);
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
