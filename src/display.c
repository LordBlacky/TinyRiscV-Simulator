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

void sendCommand (int32_t command);
void createDisplay ();
void deleteDisplay ();

//---------------------------------------------


//------------ DEFINE DISPLAY -----------------

#define RESOLUTION 132*64
#define COLS 132
#define PAGES 8

typedef struct Display {
	char pixels[RESOLUTION];
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

}

void deleteDisplay () {

	free(display);

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
		case 0x00: switch(data) {
			default: break;
		}; break;
		case 0xC0: switch(data) {
			default: break;
		}; break;
		default: break;
	};
    
}
