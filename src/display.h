/*
* TinyRiscV-Simulator 2024
* ===========================
*
* Project: https://github.com/LordBlacky/TinyRiscV-Simulator
*
*/

#ifndef DISPLAY_H_
#define DISPLAY_H_

#include<stdint.h>

// DISPLAY INTERFACE

void sendCommand (int32_t command);

void createDisplay ();

void deleteDisplay ();

char *getPixels ();

#endif
