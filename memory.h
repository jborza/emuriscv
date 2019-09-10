#pragma once
#include "state.h"



word* fetch_next_word(State* state);

word read_word(State* state, word address);

word read_halfword_signed(State* state, word address);

word read_halfword_unsigned(State* state, word address);

word read_byte_signed(State* state, word address);

word read_byte_unsigned(State* state, word address);

void write_word(State* state, word address, word value);

void write_halfword(State* state, word address, halfword value);

void write_byte(State* state, word address, byte value);