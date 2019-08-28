#pragma once
typedef struct InstructionAny {
	int opcode : 7;
	int data : 25;
} InstructionAny;

// U-type
typedef struct InstructionU {
	int opcode : 7;
	int rd : 5;
	int data : 20;
} InstructionU;

//I immediate format
typedef struct InstructionI {
	int opcode : 7;
	int rd : 5;
	int func : 3;
	int rs1 : 5;
	int imm : 12;
} InstructionI;

//I immediate format
typedef struct InstructionIShift {
	int opcode : 7;
	int rd : 5;
	int func : 3;
	int rs1 : 5;
	int shamt : 5;
	int imm : 7;
} InstructionIShift;

//R-type
typedef struct InstructionR {
	int opcode : 7;
	int rd : 5;
	int funct3 : 3;
	int rs1 : 5;
	int rs2 : 5;
	int funct7 : 3;
} InstructionR;

//S-type
typedef struct InstructionS {
	int opcode : 7;
	int imm1 : 5;
	int funct3 : 3;
	int rs1 : 5;
	int rs2 : 5;
	int imm2 : 7;
} InstructionS;

//B-type
typedef struct InstructionB {
	int opcode : 7;
	int rd : 5;
	int funct3 : 3;
	int rs1 : 5;
	int rs2 : 5;
	int imm : 3;
} InstructionB;

typedef struct StoreOffset {
	int nothing : 20;
	int offset : 12;
} StoreOffset;