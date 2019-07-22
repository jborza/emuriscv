#pragma once
#include "state.h"
word* fetch_next_word(State* state);

word read_word(State* state, word address);

halfword read_halfword(State* state, word address);

byte read_byte(State* state, word address);

void write_word(State* state, word address, word value);

void write_halfword(State* state, word address, halfword value);

void write_byte(State* state, word address, byte value);