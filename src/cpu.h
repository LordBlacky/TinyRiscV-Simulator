/*
* TinyRiscV-Simulator 2024
* ===========================
*
* Project: https://github.com/LordBlacky/TinyRiscV-Simulator
*
*/

#ifndef CPU_H_
#define CPU_H_

#include<stdint.h>
#include<pthread.h>

// CPU INTERFACE

typedef struct Memory {
	int32_t *data;
	int32_t size;
	uint8_t GPIO_IN;
	uint8_t GPIO_OUT;
	int32_t I2C_REST;
	int32_t DISPLAY;
} Memory;

typedef struct SharedMemory {
	Memory *mem;
	pthread_mutex_t mutex;
} SharedMemory;

typedef struct Register {
	int32_t *data;
	int32_t size;
} Register;

typedef enum CommandType {
	EMPTY,ADD,SUB,AND,OR,XOR,SLT,SLTU,SRA,SRL,SLL,MUL,SLLI,
	ADDI,ANDI,ORI,XORI,SLTI,SLTIU,SRAI,SRLI,LUI,AUIPC,
	LW,SW,BEQ,BNE,BLT,BGE,BLTU,BGEU,JAL,JALR,FLAG,
	//pseudo instructions
	NOP,LI,LA,MV,NOT,NEG,SEQZ,SNEZ,SLTZ,SGTZ,BEQZ,BNEZ,BLEZ,BGEZ,BLTZ,BGTZ,
	BGT,BLE, BGTU, BLEU, J,JR,RET,CALL
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

typedef struct CPU {
	Register *reg;
	SharedMemory *shared;
	Program *pgrm;
} CPU;

typedef struct CPUargs {
	CPU *cpu;
	int lifetime;
} CPUargs;

typedef struct IOargs {
	CPU *cpu;
	int32_t baseAddr; // NO LONGER IN USE, SEE FIRST CODE SECTION INSTEAD
} IOargs;

void runCommand (CPU *cpu);

#endif
