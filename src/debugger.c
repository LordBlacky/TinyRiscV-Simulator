/*
* TinyRiscV-Simulator 2024
* ===========================
*
* Project: https://github.com/LordBlacky/TinyRiscV-Simulator
*
*/

//------------ REQUIRED FUNCTIONS -------------

#include<stdio.h>
#include<stdlib.h>
#include<stdint.h>
#include "cpu.h"
#include "display.h"

void *startDebugger (void *args);

//---------------------------------------------

void *startDebugger (void *args) {

	CPU *cpu = (CPU *)args;
	printf("Started running Debugger\n");

	// PLACE CODE HERE
	//
	// USEFULL FUNCTIONS
	//
	// char *getPixels (); -> Display
	//
	// void runCommand (CPU *cpu); -> CPU
	//

	printf("Stopped running Debugger\n");
	return NULL;

}
