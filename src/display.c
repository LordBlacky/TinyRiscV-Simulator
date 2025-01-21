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

	

}
