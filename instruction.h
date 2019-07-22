#pragma once
typedef struct InstructionAny {
	int opcode : 7;
	int data : 25;
} InstructionAny;

typedef struct InstructionU {
	int opcode : 7;
	int rd : 5;
	int data : 20;
} InstructionU;