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
		printf("ERROR: No valid memory address for write access 0 / %d / %d\n",addr,mem->size);
	}

}

void wR (Register *reg, int32_t addr, int32_t data) {

	if (addr < reg->size && addr > 0) {
		(reg->data)[addr] = data;
	} else {
		printf("ERROR: No valid register address for write access 0 / %d / %d\n",addr,reg->size);
	}

}

int32_t rM (Memory *mem, int32_t addr) {

	if (addr < mem->size && addr >= 0) {
		return (mem->data)[addr];
	} else {
		printf("ERROR: No valid memory address for read access 0 / %d / %d\n",addr,mem->size);
		return 0;
	}

}

int32_t rR (Register *reg, int32_t addr) {

	if (addr < reg->size && addr > 0) {
		return (reg->data)[addr];
	} else {
		printf("ERROR: No valid register address for read access 0 / %d / %d",addr,reg->size);
		return 0;
	}

}

typedef struct Command {
	void (* func)(int32_t,int32_t,int32_t);
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

int main (int argc, char **argv) {

	Register *reg = createRegister(32);
	Memory *mem = createMemory(10000);
	Program *pgrm = createProgram(10000);

	freeRegister(reg);
	freeMemory(mem);
	freeProgram(pgrm);
	return 0;

}

