#pragma once
#include "types.h"
#include "state.h"

#define CSR_USTATUS 0x000
#define CSR_MTVEC 0x305
#define CSR_MSCRATCH 0x340
//Machine Exception Program Counter
//When a trap is taken into M-mode,mepc is written with the virtual address of the instruction that encountered the exception. 
#define CSR_MEPC 0x341
#define CSR_MTCAUSE 0x342
#define CSR_MTVAL 0x343
#define CSR_MIP 0x344

//supervisor address translation and protection
//This register holds the physical page number (PPN) of the root page table, i.e., its supervisor physical addressdivided by 4 KiB;  an address space identifier (ASID), which facilitates address-translation fenceson a per-address-space basis;  and the MODE field,  which selects the current address-translation scheme.
#define CSR_SATP 0x180 

//supervisor trap vector
#define CSR_STVEC 0x105

#define CSR_PMPCFG0 0x3A0
#define CSR_PMPCFG1 0x3A1
#define CSR_PMPCFG2 0x3A2
#define CSR_PMPCFG3 0x3A3
#define CSR_PMPADDR0 0x3B0
#define CSR_PMPADDR1 0x3B1
#define CSR_PMPADDR2 0x3B2
#define CSR_PMPADDR3 0x3B3
#define CSR_PMPADDR4 0x3B4
#define CSR_PMPADDR5 0x3B5
#define CSR_PMPADDR6 0x3B6
#define CSR_PMPADDR7 0x3B7
#define CSR_PMPADDR8 0x3B8
#define CSR_PMPADDR9 0x3B9
#define CSR_PMPADDR10 0x3BA
#define CSR_PMPADDR11 0x3BB
#define CSR_PMPADDR12 0x3BC
#define CSR_PMPADDR13 0x3BD
#define CSR_PMPADDR14 0x3BE
#define CSR_PMPADDR15 0x3BF

word read_csr(State *state, int csr);
void write_csr(State *state, int csr, word value);