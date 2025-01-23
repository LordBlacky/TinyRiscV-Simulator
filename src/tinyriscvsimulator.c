/*
* TinyRiscV-Simulator 2024
* ===========================
*
* Project: https://github.com/LordBlacky/TinyRiscV-Simulator
*
*/

#include<stdint.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<pthread.h>
#include "display.h"
#include "cpu.h"
#include "debugger.h"

// UDP SOCKET FOR I/O AND I2C DEVICES ---
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>

#define PORT 50000
#define BUFFER_SIZE 1024
#define GPIO_ADDR_IN 0x100001
#define GPIO_ADDR_OUT 0x100000
#define I2C_ADDR_MIN 0x100004
#define I2C_ADDR_MAX 0x100084
#define DISPLAY_ADDR 0x100040
// -----------------------

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
			mem->size = size*4;
			mem->GPIO_IN = 0;
			mem->GPIO_OUT = 0;
			mem->DISPLAY = 0;
			mem->I2C_REST = 0;
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

	int8_t *bytePtr = (int8_t *)(mem->data);
	int32_t *byteAddr = (int32_t *)(bytePtr + addr);
	
	if (addr < mem->size && addr >= 0) {
		if (addr == GPIO_ADDR_OUT) {
			mem->GPIO_OUT = data & 0xFF;
		} else if (addr == GPIO_ADDR_IN) {
			printf("ERROR: Writing to GPIO_IN not possible\n");
		} else if (addr >= I2C_ADDR_MIN && addr <= I2C_ADDR_MAX ) {
			if (addr == DISPLAY_ADDR) {
				mem->DISPLAY = data;
			} else {
				mem->I2C_REST = data;
			}
		} else {
			*byteAddr = data;
		}
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

	int8_t *bytePtr = (int8_t *)(mem->data);
	int32_t *byteAddr = (int32_t *)(bytePtr + addr);

	if (addr < mem->size && addr >= 0) {
		if (addr == GPIO_ADDR_IN) {
			return (int32_t)mem->GPIO_IN;
		} else if (addr == GPIO_ADDR_OUT) {
			printf("ERROR: Reading from GPIO_OUT not possible\n");
			return 0;
		} else if (addr >= I2C_ADDR_MIN && addr <= I2C_ADDR_MAX){
			if (addr == DISPLAY_ADDR) {
				return mem->DISPLAY;
			} else {
				return mem->I2C_REST;
			}
		} else {
			return *byteAddr;
		}
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

void executeExpansion(CommandType type, int32_t arg1, int32_t arg2, int32_t arg3, Register *reg, Memory *mem, Program *pgrm) {

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
			wR(reg,rd,(rM(mem,rR(reg,rs2) + rs1)));
			pgrm->pc += 4;
			break;
		case SW:
			wM(mem,rR(reg,rs2) + rs1,rR(reg,rd));
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
			pgrm->pc = ((rR(reg,rs2) + rs1) & 0xfffffffe);
			break;

		//also add supported pseudo instruction
		case NOP:
			break;
		case LI:
			executeExpansion(ADDI, rd, 0, rs1, reg, mem, pgrm);
			break;
		case LA:
			printf("ERROR: LA instruction is currently not supported\n");
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
			executeExpansion(JALR, 0, rd, 0, reg, mem, pgrm);
			break;
		case RET:
			executeExpansion(JALR, 0, 1, 0, reg, mem, pgrm);
			break;
		case CALL:
			executeExpansion(JAL, 1, rd, 0, reg, mem, pgrm);
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
		sendCommand(cpu->shared->mem->DISPLAY);
		pthread_mutex_unlock(&cpu->shared->mutex);
	} else {
		executeCommand(cmd,cpu->reg,cpu->shared->mem,cpu->pgrm);	
	}

}

void *runCPU (void *args) {

	printf("Started running CPU\n");

	CPU *cpu = ((CPUargs *)args)->cpu;
	int lifetime = ((CPUargs *)args)->lifetime;

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

	printf("Stopped running CPU\n");
	return NULL;

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

void *runIOConnector (void *args) {

	printf("Started running IOConnector\n");

	CPU *cpu = ((IOargs *)args)->cpu;
	int32_t baseAddr = ((IOargs *)args)->baseAddr;

	int sockfd;
	char buffer[BUFFER_SIZE];
	struct sockaddr_in servaddr, cliaddr;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("ERROR: Cannot create socket\n");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(PORT);

	if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		printf("ERROR: Cannot bind socket\n");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	socklen_t len; 
	int n;
	len = sizeof(cliaddr);
	const char *ack = "Message received";

	while (1) {
		n = recvfrom(sockfd, (char *)buffer, BUFFER_SIZE, 0, (struct sockaddr *)&cliaddr, &len);
		if (n < 0) {
			printf("ERROR: recvfrom() failed\n");
			continue;
		}
		buffer[n] = '\0';
		sendto(sockfd, ack, strlen(ack), MSG_CONFIRM, (const struct sockaddr *)&cliaddr, len);

		if (strcmp(buffer,"EXIT") == 0) {

			break;

		} else {

			pthread_mutex_lock(&cpu->shared->mutex);

			// wM(cpu->shared->mem,baseAddr,atoi(buffer));
			cpu->shared->mem->GPIO_IN = atoi(buffer);
			// printf("%d\n",cpu->shared->mem->GPIO_IN);

			pthread_mutex_unlock(&cpu->shared->mutex);

		}

	}

	close(sockfd);
	printf("Stopped running IOConnector\n");
	return NULL;

}

void runSimulation (int memsize, int pgrmsize, int lifetime, char *file, int baseAddr) {

	pthread_t runner, io;

	CPU *cpu = createCPU(memsize, pgrmsize);

	createDisplay();

	readProgram(cpu,file);

	CPUargs *runnerArgs = malloc(sizeof(CPUargs));
	runnerArgs->cpu = cpu;
	runnerArgs->lifetime = lifetime;

	IOargs *ioArgs = malloc(sizeof(IOargs));
	ioArgs->cpu = cpu;
	ioArgs->baseAddr = baseAddr;

	/*
	if (pthread_create(&runner, NULL, runCPU, runnerArgs)) {

		printf("ERROR: Failed to create Runner Thread\n");
		exit(EXIT_FAILURE);

	}
	*/ 

	if (pthread_create(&runner,NULL,startDebugger,cpu)) {

		printf("ERROR: Failed to create Runner Thread\n");
		exit(EXIT_FAILURE);

	}


	if (pthread_create(&io, NULL, runIOConnector, ioArgs)) {

		printf("ERROR: Failed to create I/O Thread\n");
		exit(EXIT_FAILURE);

	}

	if (pthread_join(io, NULL)) {

		printf("ERROR: joining thread I/O\n");
		exit(EXIT_FAILURE);

	}

	if (pthread_join(runner, NULL)) {

		printf("ERROR: joining thread Runner\n");
		exit(EXIT_FAILURE);

	}

	freeCPU(cpu);
	deleteDisplay();
	free(runnerArgs);
	free(ioArgs);

}

int main (int argc, char **argv) {

	runSimulation(10000000,10000000,1000000,argv[1],1000);

	return 0;

}

