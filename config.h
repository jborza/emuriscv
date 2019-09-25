#pragma once
//#define RUN_TESTS
#define PRINT_OPCODES

#define RUN_LINUX
#define RUN_LINUX_VERBOSE
//whether to build flat device tree or load from a blobd/
#define BUILD_REAL_FDT
#define VM_MEMORY_SIZE 64 * 1024 * 1024

//define to allow for 
#define EXTENSION_A
#define EXTENSION_ZICSR
#define EXTENSION_M

extern int print_verbose;