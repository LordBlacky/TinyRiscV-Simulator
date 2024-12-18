/*
* TinyRiscV-Simulator 2024
* ===========================
*
* Project: https://github.com/LordBlacky/TinyRiscV-Simulator
*
*/

#include <stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

typedef struct Memory {
	int32_t *data;
	int32_t size;
} Memory;

typedef struct Register {
	int32_t *data;
	int32_t size;
} Register;

Memory *createMemory (int32_t size) {

	Memory *mem = malloc(sizeof(Memory));
	if (mem == NULL) {
		printf("ERROR: Cannot allocate memory\n");
		return NULL;
	} else {
		mem->data = malloc(sizeof(int32_t)*size);
		if (mem->data == NULL) {
			free(mem);
			printf("ERROR: Cannot allocate memory\n");
			return NULL;
		} else {
			mem->size = size;
		}
	}
	return mem;

}

Register *createRegister (int32_t size) {

	Register *reg = malloc(sizeof(Register));
	if (reg == NULL) {
		printf("ERROR: Cannot allocate register\n");
		return NULL;
	} else {
		reg->data = malloc(sizeof(int32_t)*size);
		if (reg->data == NULL) {
			free(reg);
			printf("ERROR: Cannot allocate register\n");
			return NULL;
		} else {
			reg->size = size;
			(reg->data)[0] = 0;
		}
	}
	return reg;

}

void freeMemory (Memory *mem) {

	free(mem->data);
	free(mem);

}

void freeRegister (Register *reg) {

	free(reg->data);
	free(reg);

}

void wM (Memory *mem, int32_t addr, int32_t data) {
	
	if (addr < mem->size && addr >= 0) {
		(mem->data)[addr] = data;
	} else {
		printf("ERROR: No valid memory address for write access 0 / %d / %d\n",addr,mem->size-1);
	}

}

void wR (Register *reg, int32_t addr, int32_t data) {

	if (addr < reg->size && addr >= 0) {
		if (addr != 0) {
			(reg->data)[addr] = data;
		}
	} else {
		printf("ERROR: No valid register address for write access 0 / %d / %d\n",addr,reg->size-1);
	}

}

int32_t rM (Memory *mem, int32_t addr) {

	if (addr < mem->size && addr >= 0) {
		return (mem->data)[addr];
	} else {
		printf("ERROR: No valid memory address for read access 0 / %d / %d\n",addr,mem->size-1);
		return 0;
	}

}

int32_t rR (Register *reg, int32_t addr) {

	if (addr < reg->size && addr >= 0) {
		if (addr == 0) {
			return 0;
		} else {
			return (reg->data)[addr];
		}
	} else {
		printf("ERROR: No valid register address for read access 0 / %d / %d",addr,reg->size-1);
		return 0;
	}

}

typedef enum CommandType {
	ADD,SUB,AND,OR,XOR,SLT,SLTU,SRA,SRL,SLL,MUL,SLLI,
	ADDI,ANDI,ORI,XORI,SLTI,SLTIU,SRAI,SRLI,LUI,AUIPC,
	LW,SW,BEQ,BNE,BLT,BGE,BLTU,BGEU,JAL,JALR,FLAG,EMPTY
} CommandType;

typedef struct Command {
	CommandType type;
	int32_t a;
	int32_t b;
	int32_t c;
} Command;

typedef struct Program {
	int32_t pc;
	Command *addr;
	int32_t size;
} Program;

Program *createProgram (int32_t size) {
	
	Program *pgrm = malloc(sizeof(Program));
	if (pgrm == NULL) {
		printf("ERROR: Cannot allocate program\n");
		return NULL;
	} else {
		pgrm->size = size;
		pgrm->pc = 0;
		pgrm->addr = malloc(sizeof(Command)*size);
		if (pgrm->addr == NULL) {
			free(pgrm);
			printf("ERROR: Cannot allocate program\n");
			return NULL;
		} else {
			return pgrm;
		}
	}

}

void freeProgram (Program *pgrm) {

	free(pgrm->addr);
	free(pgrm);

}

void addCommand (Program *pgrm, int32_t line, CommandType type, int32_t a, int32_t b, int32_t c) {

	if (line < 0 || line >= pgrm->size ) {
		printf("ERROR: addCommand - line is not in between 0 / %d / %d\n",line,pgrm->size-1);
		return;
	} else {
		(pgrm->addr)[line].type = type;
		(pgrm->addr)[line].a = a;
		(pgrm->addr)[line].b = b;
		(pgrm->addr)[line].c = c;
	}

}

Command getCommand (Program *pgrm) {

	return (pgrm->addr)[pgrm->pc/4];

}

void executeCommand (Command cmd, Register *reg, Memory *mem, Program *pgrm) {

	CommandType type = cmd.type;
	int32_t rd = cmd.a;
	int32_t rs1 = cmd.b;
	int32_t rs2 = cmd.c;
	// printf("Executing command: %d,%d,%d,%d\n",type,rd,rs1,rs2);
	
	switch(type) {

		case ADD:
			wR(reg,rd,(rR(reg,rs1) + rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case SUB:
			wR(reg,rd,(rR(reg,rs1) - rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case AND:
			wR(reg,rd,(rR(reg,rs1) & rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case OR:
			wR(reg,rd,(rR(reg,rs1) | rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case XOR:
			wR(reg,rd,(rR(reg,rs1) ^ rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case FLAG:
			pgrm->pc += 4;
			break;
		case EMPTY:
			pgrm->pc += 4;
			break;
		case SLT:
			wR(reg,rd,(rR(reg,rs1) < rR(reg,rs2) ? 1 : 0));
			pgrm->pc += 4;
			break;
		case SLTU:
			wR(reg,rd,((uint32_t)rR(reg,rs1) < (uint32_t)rR(reg,rs2) ? 1 : 0));
			pgrm->pc += 4;
			break;
		case SRA:
			wR(reg,rd,(rR(reg,rs1) >> (rR(reg,rs2)&31)));
			pgrm->pc += 4;
			break;
		case SRL:
			wR(reg,rd,((uint32_t)rR(reg,rs1) >> (rR(reg,rs2)&31)));
			pgrm->pc += 4;
			break;
		case SLL:
			wR(reg,rd,(rR(reg,rs1) << (rR(reg,rs2)&31)));
			pgrm->pc += 4;
			break;
		case MUL:
			wR(reg,rd,(rR(reg,rs1) * rR(reg,rs2)));
			pgrm->pc += 4;
			break;
		case SLLI:
			wR(reg,rd,(rR(reg,rs1) << rs2));
			pgrm->pc += 4;
			break;
		case ADDI:
			wR(reg,rd,(rR(reg,rs1) + rs2));
			pgrm->pc += 4;
			break;
		case ANDI:
			wR(reg,rd,(rR(reg,rs1) & rs2));
			pgrm->pc += 4;
			break;
		case ORI:
			wR(reg,rd,(rR(reg,rs1) | rs2));
			pgrm->pc += 4;
			break;
		case XORI:
			wR(reg,rd,(rR(reg,rs1) ^ rs2));
			pgrm->pc += 4;
			break;
		case SLTI:
			wR(reg,rd,(rR(reg,rs1) < rs2 ? 1 : 0));
			pgrm->pc += 4;
			break;
		case SLTIU:
			wR(reg,rd,((uint32_t)rR(reg,rs1) < (uint32_t)rs2 ? 1 : 0));
			pgrm->pc += 4;
			break;
		case SRAI:
			wR(reg,rd,(rR(reg,rs1) >> (rs2 & 31)));
			pgrm->pc += 4;
			break;
		case SRLI:
			wR(reg,rd,((uint32_t)rR(reg,rs1) >> (rs2 & 31)));
			pgrm->pc += 4;
			break;
		case LUI:
			wR(reg,rd,(rs1 << 12));
			pgrm->pc += 4;
			break;
		case AUIPC:
			wR(reg,rd,(pgrm->pc + (rs1 << 12)));
			pgrm->pc += 4;
			break;
		case LW:
			wR(reg,rd,(rM(mem,rR(reg,rs1) + rs2)));
			pgrm->pc += 4;
			break;
		case SW:
			wM(mem,rR(reg,rs1) + rs2,rR(reg,rd));
			pgrm->pc += 4;
			break;
		case BEQ:
			pgrm->pc += (rR(reg,rd) == rR(reg,rs1) ? rs2 : 4);
			break;
		case BNE:
			pgrm->pc += (rR(reg,rd) != rR(reg,rs1) ? rs2 : 4);
			break;
		case BLT:
			pgrm->pc += (rR(reg,rd) < rR(reg,rs1) ? rs2 : 4);
			break;
		case BGE:
			pgrm->pc += (rR(reg,rd) >= rR(reg,rs1) ? rs2 : 4);
			break;
		case BLTU:
			pgrm->pc += ((uint32_t)rR(reg,rd) < (uint32_t)rR(reg,rs1) ? rs2 : 4);
			break;
		case BGEU:
			pgrm->pc += ((uint32_t)rR(reg,rd) >= (uint32_t)rR(reg,rs1) ? rs2 : 4);
			break;
		case JAL:
			wR(reg,rd,(pgrm->pc + 4));
			pgrm->pc += rs1;
			break;
		case JALR:
			wR(reg,rd,(pgrm->pc + 4));
			pgrm->pc = ((rR(reg,rs1) + rs2) & 0xfffffffe);
			break;
		default:
			break;

	}

}

void printRegister (Register *reg) {

	printf("=========================================================================\n");

	printf("CURRENT REGISTER VIEW\n");
	
	printf("-------------------------------------------------------------------------\n");

	printf("x0: %d | x1: %d | x2: %d | x3: %d | x4: %d | x5: %d\n"
	,reg->data[0],reg->data[1],reg->data[2],reg->data[3],reg->data[4],reg->data[5]);
	
	printf("x6: %d | x7: %d | x8: %d | x9: %d | x10: %d | x11: %d\n"
	,reg->data[6],reg->data[7],reg->data[8],reg->data[9],reg->data[10],reg->data[11]);
	
	printf("x12: %d | x13: %d | x14: %d | x15: %d | x16: %d | x17: %d\n"
	,reg->data[12],reg->data[13],reg->data[14],reg->data[15],reg->data[16],reg->data[17]);
	
	printf("x18: %d | x1: %d | x19: %d | x20: %d | x21: %d | x22: %d\n"
	,reg->data[18],reg->data[19],reg->data[20],reg->data[21],reg->data[22],reg->data[23]);
	
	printf("x24: %d | x25: %d | x26: %d | x27: %d | x28: %d | x29: %d\n"
	,reg->data[24],reg->data[25],reg->data[26],reg->data[27],reg->data[28],reg->data[29]);
	
	printf("x30: %d | x31: %d\n"
	,reg->data[30],reg->data[31]);
	
	printf("=========================================================================\n");
	
}

int main (int argc, char **argv) {

	Register *reg = createRegister(32);
	Memory *mem = createMemory(10000);
	Program *pgrm = createProgram(10000);

	addCommand(pgrm,0,ADDI,1,0,1);
	addCommand(pgrm,1,ADDI,1,1,2);
	addCommand(pgrm,2,ADDI,1,1,-2);
	addCommand(pgrm,3,JAL,0,-8,0);

	int j = 0;
	while (j++ < 150000000) {
		executeCommand(getCommand(pgrm),reg,mem,pgrm);
	}
	printRegister(reg);

	freeRegister(reg);
	freeMemory(mem);
	freeProgram(pgrm);
	return 0;

}

