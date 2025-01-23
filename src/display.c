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
#include "display.h"

//---------------------------------------------


//------------ DEFINE DISPLAY -----------------

typedef struct Display {
	char pixels[PAGES*8][COLS+1];
	int pages;
	int cols;
	int pageIDX;
	int colIDX;
	int power;
} Display;

Display *display;

//---------------------------------------------

void createDisplay () {

	display = malloc(sizeof(Display));
	display->power = 0;
	display->cols = COLS;
	display->pages = PAGES;
	display->colIDX = 0;
	display->pageIDX = 0;
	for (int i = 0; i< PAGES*8; i++) {
		for (int j = 0; i < COLS; i++) {
			display->pixels[i][j] = ' ';
		}
		display->pixels[i][COLS] = '\00';
	}

}

void deleteDisplay () {

	free(display);

}

void runDisplayCommand (uint8_t data) {



}

void runUpdate (uint8_t data) {

	

}

char (*getPixels()) [COLS] {

	return &(display->pixels[0]);

}

void sendCommand (int32_t command) {

    uint8_t fourthByte = (command >> 0) & 0xFF;
    uint8_t thirdByte = (command >> 8) & 0xFF;
    uint8_t secondByte = (command >> 16) & 0xFF;
    uint8_t mask = (command >> 24) & 0xFF;
	uint8_t identifier;
	uint8_t data;

	switch (mask) {
		case 0x03: identifier = thirdByte; data = fourthByte; break;
		case 0x06: identifier = secondByte; data = thirdByte; break;
		case 0x05: identifier = secondByte; data = fourthByte; break;
		default: break;
	};

	switch (identifier) {
		case 0x00: runDisplayCommand(data); break;
		case 0xC0: runUpdate(data); break;
		default: break;
	};
    
}
