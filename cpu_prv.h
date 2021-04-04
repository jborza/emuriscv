#pragma once
#define get_field(reg, mask) (((reg) & (mask)) / ((mask) & ~((mask) << 1)))
#define set_field(reg, mask, val) (((reg) & ~(mask)) | (((val) * ((mask) & ~((mask) << 1))) & (mask)))

void mret(State* state, word* instruction);
void sret(State* state, word* instruction);
void uret(State* state, word* instruction);
void sfence_vma(State* state, word* instruction);
void wfi(State* state, word* instruction);