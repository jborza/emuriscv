#pragma once
//#define RUN_TESTS
//#define PRINT_OPCODES

//define to override "opcode" printing breakpoint
//#define PRINT_OPCODES_ALWAYS

#define RUN_LINUX
#define RUN_LINUX_VERBOSE
//whether to build flat device tree or load from a blobd/
#define BUILD_REAL_FDT
#define VM_MEMORY_SIZE 96 * 1024 * 1024

//define to allow for 
#define EXTENSION_A
#define EXTENSION_ZICSR
#define EXTENSION_M

//note: "earlycon=sbi" 
#define LINUX_CMDLINE "debug keep_bootcon bootmem_debug earlycon=sbi"

//"linux/vmlinux-smalldebug-rv32ia.bin"
#define LINUX_BINARY "linux/vmlinux-5.10.bin"

//if defined, console output goes to console.out in the working directory
#define OUTPUT_CONSOLE_TO_FILE

#define ENABLE_CONSOLE

extern int print_verbose;

