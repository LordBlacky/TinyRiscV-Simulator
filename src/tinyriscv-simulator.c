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
#include<pthread.h>

typedef struct Memory {
	int32_t *data;
	int32_t size;
} Memory;

typedef struct SharedMemory {
	Memory *mem;
	pthread_mutex_t mutex;
} SharedMemory;

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

SharedMemory *createSharedMemory (int32_t size) {

	SharedMemory *shared = malloc(sizeof(SharedMemory));
	if (shared == NULL) {
		printf("ERROR: Cannot allocate shared memory\n");
		return NULL;
	} else {
		Memory *mem = createMemory(size);
		shared->mem = mem;
		pthread_mutex_init(&shared->mutex, NULL);
	}
	return shared;

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

void freeSharedMemory (SharedMemory *shared) {

	freeMemory(shared->mem);
	pthread_mutex_destroy(&shared->mutex);
	free(shared);

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
	EMPTY,ADD,SUB,AND,OR,XOR,SLT,SLTU,SRA,SRL,SLL,MUL,SLLI,
	ADDI,ANDI,ORI,XORI,SLTI,SLTIU,SRAI,SRLI,LUI,AUIPC,
	LW,SW,BEQ,BNE,BLT,BGE,BLTU,BGEU,JAL,JALR,FLAG,
	//pseudo instructions
	NOP,LI,LA,MV,NOT,NEG,SEQZ,SNEZ,SLTZ,SGTZ,BEQZ,BNEZ,BLEZ,BGEZ,BLTZ,BGTZ,
	BGT,BLE, BGTU, BLEU, J,JR,RET
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

void executeCommand (Command cmd, Register *reg, Memory *mem, Program *pgrm);

void executeExpansion(CommandType type, int32_t arg1, int32_t arg2, int32_t arg3, Register *reg, Memory *mem, Program *pgrm)
{
	Command *cmd = malloc(sizeof(Command));
	cmd->type = type;
	cmd->a = arg1;
	cmd->b = arg2;
	cmd->c = arg3;
	executeCommand(*cmd, reg, mem, pgrm);
	free(cmd);
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

		//also add supported pseudo instruction
		case NOP:
			break;
		case LI:
			executeExpansion(ADDI, rd, 0, rs1, reg, mem, pgrm);
			break;
		case LA:
			printf("LA instruction is currently not supported");
			break;
		case MV:
			executeExpansion(ADDI, rd, rs1, 0, reg, mem, pgrm);
			break;
		case NOT:
			executeExpansion(XORI, rd, rs1, 0, reg, mem, pgrm);
			break;
		case NEG:
			executeExpansion(SUB, rd, 0, 0, reg, mem, pgrm);
			break;
		case SEQZ:
			executeExpansion(SLTIU, rd, rs1, 1, reg, mem, pgrm);
			break;
		case SNEZ:
			executeExpansion(SLTU, rd, 0, rs1, reg, mem, pgrm);
			break;
		case SLTZ:
			executeExpansion(SLT, rd, rs1, 0, reg, mem, pgrm);
			break;
		case SGTZ:
			executeExpansion(SLT, rd, 0, rs1, reg, mem, pgrm);
			break;
		case BEQZ:
			executeExpansion(BEQ, rd, 0, rs1, reg, mem, pgrm);
			break;
		case BNEZ:
			executeExpansion(BNE, rd, 0, rs1, reg, mem, pgrm);
			break;
		case BLEZ:
			executeExpansion(BGE, 0, rd, rs1, reg, mem, pgrm);
			break;
		case BGEZ:
			executeExpansion(BGE, rd, 0, rs1, reg, mem, pgrm);
			break;
		case BLTZ:
			executeExpansion(BLT, rd, 0, rs1, reg, mem, pgrm);
			break;
		case BGTZ:
			executeExpansion(BLT, 0, rd, rs1, reg, mem, pgrm);
			break;
		case BGT:
			executeExpansion(BLT, rs1, rd, rs2, reg, mem, pgrm);
			break;
		case BLE:
			executeExpansion(BGE, rs1, rd, rs2, reg, mem, pgrm);
			break;
		case  BGTU:
			executeExpansion(BLTU, rs1, rd, rs2, reg, mem, pgrm);
			break;
		case BLEU:
			executeExpansion(BLTU, rs1, rd, rs2, reg, mem, pgrm);
			break;
		case J:
			executeExpansion(JAL, 0, rd, 0, reg, mem, pgrm);
			break;
		case JR:
			executeExpansion(JAL, 1, rd, 0, reg, mem, pgrm);
			break;
		case RET:
			executeExpansion(JALR, 0, 1, 0, reg, mem, pgrm);
			break;
		default:
			break;

	}

}

void printRegister(Register *reg) {

    printf("=====================================================================================\n");
    printf("CURRENT REGISTER VIEW\n");
    printf("-------------------------------------------------------------------------------------\n");

    for (int i = 0; i < reg->size; i += 4) {
        printf("x%-2d: %12d | x%-2d: %12d | x%-2d: %12d | x%-2d: %12d\n",
               i, reg->data[i],
               i + 1, reg->data[i + 1],
               i + 2, reg->data[i + 2],
               i + 3, reg->data[i + 3]);
    }

    printf("=====================================================================================\n");

}

typedef struct CPU {
	Register *reg;
	SharedMemory *shared;
	Program *pgrm;
} CPU;

CPU *createCPU (int32_t memsize, int32_t pgrmsize) {

	CPU *cpu = malloc(sizeof(CPU));
	if (cpu == NULL) {
		printf("ERROR: Failed to allocate cpu\n");
		return NULL;
	} else {
		cpu->reg = createRegister(32);
		cpu->shared = createSharedMemory(memsize);
		cpu->pgrm = createProgram(pgrmsize);
	}
	return cpu;

}

void freeCPU (CPU *cpu) {

	freeRegister(cpu->reg);
	freeSharedMemory(cpu->shared);
	freeProgram(cpu->pgrm);
	free(cpu);

}

void runCommand (CPU *cpu) {

	Command cmd = getCommand(cpu->pgrm);
	if (cmd.type == LW || cmd.type == SW) {
		pthread_mutex_lock(&cpu->shared->mutex);
		executeCommand(cmd,cpu->reg,cpu->shared->mem,cpu->pgrm);
		pthread_mutex_unlock(&cpu->shared->mutex);
	} else {
		executeCommand(cmd,cpu->reg,cpu->shared->mem,cpu->pgrm);	
	}

}

void runCPU (CPU *cpu, int lifetime) {

	int instnum = 0;
	if (lifetime != -1) {
		while (instnum++ < lifetime) {
			runCommand(cpu);
		}
	} else {
		while (1) {
			runCommand(cpu);
		}
	}

}

void readProgram (CPU *cpu, char *name) {

	FILE *file = fopen(name,"r");
	if (file == NULL) {
		printf("ERROR: cannot open provided file\n");
	} else {
		char *line = NULL;
		size_t len = 0;
		ssize_t read;
		int lnum = 0;
		while ((read = getline(&line,&len,file)) != -1) {
			CommandType type;
			int32_t a;
			int32_t b;
			int32_t c;
			char *token = strtok(line," ");
			type = atoi(token);
			token = strtok(NULL," ");
			a = atoi(token);
			token = strtok(NULL," ");
			b = atoi(token);
			token = strtok(NULL,"\n");
			c = atoi(token);

			addCommand(cpu->pgrm,lnum,type,a,b,c);
			lnum++;
		}
		free(line);
		fclose(file);
	}

}

void runSimulation (int memsize, int pgrmsize, int lifetime, char *file) {

	CPU *cpu = createCPU(memsize, pgrmsize);

	readProgram(cpu,file);

	runCPU(cpu,lifetime);

	freeCPU(cpu);

}

int main (int argc, char **argv) {

	runSimulation(10000,10000,100000000,argv[1]);

	return 0;

}

