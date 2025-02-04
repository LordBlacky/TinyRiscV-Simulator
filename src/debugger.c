/*
 * TinyRiscV-Simulator 2024
 * ===========================
 *
 * Project: https://github.com/LordBlacky/TinyRiscV-Simulator
 *
 */

#include <curses.h>
#include <ncurses.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "display.h"

#define MAX_LINES 1024           // Maximum number of lines
#define MAX_LINE_LENGTH 1024     // Maximum length of a single line
#define BOTTOM_WINDOW_PADDING 70
#define INSTRUCTION_LINES 10 // Number of instructions shown when debugging
#define NUMBER_OF_INSTRUCTIONS 20

const int SLEEPTIME = 1000;
const int CYCLES_PER_SLEEP = 1000;
const int RES_X = 270;
const int RES_Y = 70;

int mem_base_addr = 0;

int nextCommand = 0;
int nextBreakpoint = 0;
int justRun = 0;

int showDisplay = 1;
int showRegister = 1;
int showMemory = 1;
int showCode = 1;
int shouldClear = 0;

// Array of pointers to hold lines
char *lines[MAX_LINES];
int line_count = 0;

// Array of integers, holding linenumbers of breakpoints
int *breakpoints;
int breakpoint_count;

//panel vars
int next_panel_y = 1;
int next_panel_x = 1;
WINDOW *win;

void print_instructions(int line) {
  // Print all lines
  int max_lines = (line + NUMBER_OF_INSTRUCTIONS) < (line_count)
                      ? (line + NUMBER_OF_INSTRUCTIONS)
                      : (line_count);
  //
  wmove(win, next_panel_y, next_panel_x);
  wprintw(win, "->");

  for (size_t i = line; i < max_lines; i++) {

    wmove(win, next_panel_y + i - line, next_panel_x + 2);
    wprintw(win, "%zu: %s", i + 1, lines[i]);
  }

  next_panel_y += NUMBER_OF_INSTRUCTIONS;
}

void printRegister(Register *reg) {
  wmove(win, next_panel_y++, next_panel_x); // Adjust the position as needed
  wprintw(win, "==============================================================="
               "======================\n");
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "CURRENT REGISTER VIEW\n");
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "---------------------------------------------------------------"
               "----------------------\n");

  int i = 0;
  for (; i < reg->size; i += 4) {
    wmove(win, next_panel_y + i / 4, next_panel_x);
    wprintw(win,
            "x%-2d: 0x%12x | x%-2d: 0x%12x | x%-2d: 0x%12x | x%-2d: 0x%12x\n",
            i, (uint32_t)reg->data[i], i + 1, (uint32_t)reg->data[i + 1], i + 2,
            (uint32_t)reg->data[i + 2], i + 3, (uint32_t)reg->data[i + 3]);
  }
  next_panel_y += i / 4;
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "==============================================================="
               "======================\n");
}

void printMemory(Memory *mem, int addr) {
  int8_t *memaddr = (int8_t *)(mem->data);
  wmove(win, next_panel_y++, next_panel_x); // Adjust the position as needed
  wprintw(win, "==============================================================="
               "==========================\n");
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "CURRENT MEMORY VIEW (LITTLE ENDIAN!)\n");
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "---------------------------------------------------------------"
               "--------------------------\n");

  int i = 0;
  for (; i < 64; i += 4) {
    wmove(win, next_panel_y + i / 4, next_panel_x);
    wprintw(win, "%-4d: 0x%12x | %-4d: 0x%12x | %-4d: 0x%12x | %-4d: 0x%12x\n",
            addr + i, (uint8_t)memaddr[addr + i], addr + i + 1,
            (uint8_t)memaddr[addr + i + 1], addr + i + 2,
            (uint8_t)memaddr[addr + i + 2], addr + i + 3,
            (uint8_t)memaddr[addr + i + 3]);
  }
  next_panel_y += i / 4;
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "GPIO-IN: 0x%x GPIO-OUT: 0x%x I2C-DISPLAY: 0x%x I2C-REST: 0x%x",
          (uint8_t)mem->GPIO_IN, (uint8_t)mem->GPIO_OUT, (uint32_t)mem->DISPLAY,
          (uint32_t)mem->I2C_REST);
  wmove(win, next_panel_y++, next_panel_x);
  wprintw(win, "==============================================================="
               "==========================\n");
}

void print_display(char (*display)[COLS + 1]) {
  wmove(win, next_panel_y, next_panel_x);
  int i;
  for (i = 0; i < PAGES * 8; i++) {
    wprintw(win, "%s\n", display[i]);
    wmove(win, i + next_panel_y, next_panel_x);
  }
  // draw box around display
  whline(win, next_panel_x - 1, COLS);
  mvwvline(win, 1, next_panel_x + COLS + 1, 0, PAGES * 8 - 1);
  next_panel_x += COLS+2;
}

void load_debug_file() {
  const char *filename = "./debugger_info.txt";
  FILE *file = fopen(filename, "r"); // Open file in text mode

  if (file == NULL) {
    perror("Error opening file");
    return;
  }

  // Read each line from the file
  char buffer[MAX_LINE_LENGTH];
  while (fgets(buffer, sizeof(buffer), file) != NULL) {
    if (line_count >= MAX_LINES) {
      fprintf(stderr, "Error: Too many lines in file. Maximum is %d.\n",
              MAX_LINES);
      break;
    }

    // Allocate memory for the line and copy it
    lines[line_count] = malloc(strlen(buffer) + 1);
    if (lines[line_count] == NULL) {
      perror("Error allocating memory");
      fclose(file);
      return;
    }
    strcpy(lines[line_count], buffer);
    line_count++;
  }

  fclose(file);

  return;
}

void read_breakpoint_info() {
  const char *filename = "breakpoint_info.txt";
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("Error opening file");
    breakpoint_count = 0;
    return;
  }

  // Allocate initial space for the array
  int capacity = 10;
  int *line_numbers = malloc(capacity * sizeof(int));
  if (line_numbers == NULL) {
    perror("Memory allocation error");
    fclose(file);
    breakpoint_count = 0;
    return;
  }

  breakpoint_count = 0;
  int num;
  while (fscanf(file, "%d", &num) == 1) {
    // Expand the array if needed
    if (breakpoint_count >= capacity) {
      capacity *= 2;
      int *temp = realloc(line_numbers, capacity * sizeof(int));
      if (temp == NULL) {
        perror("Memory reallocation error");
        free(line_numbers);
        fclose(file);
        breakpoint_count = 0;
        return;
      }
      line_numbers = temp;
    }

    line_numbers[breakpoint_count] = num;
    (breakpoint_count)++;
  }

  fclose(file);
  breakpoints = line_numbers;
}

void free_lines() {
  // Free allocated memory
  for (size_t i = 0; i < line_count; i++) {
    free(lines[i]);
  }
  free(breakpoints);
}

void init_screen() {
  initscr();
  cbreak();
  noecho();
  start_color();
  init_pair(1, COLOR_BLACK, COLOR_WHITE);
  // make curser invisible
  curs_set(0);

  win = newwin(RES_Y, RES_X, 0, 0);
  box(win, 0, 0);
  wmove(win, 1, 1);
  refresh();
  wrefresh(win);
}

void init_debugger() {
  init_screen();
  load_debug_file();
  read_breakpoint_info();

  getch();
  endwin();
}

void *threadOne(void *args) {

  CPU *cpu = (CPU *)args;

  while (1) {
    if (shouldClear) {
      wclear(win);
      shouldClear = 0;
    }
    showDisplay ? print_display(getPixels()) : NULL;
    showCode ? print_instructions(cpu->pgrm->pc / 4) : NULL;
    showRegister ? printRegister(cpu->reg) : NULL;
    showMemory ? printMemory(cpu->shared->mem, mem_base_addr) : NULL;
    next_panel_x = 1;
    next_panel_y = 1;

    box(win, 0, 0);
    refresh();
    wrefresh(win);
    usleep(10000);
  }

  return NULL;
}

void *threadTwo(void *args) {

  CPU *cpu = (CPU *)args;

  while (1) {

    char in = getch();
    switch (in) {
    case 'n':
      nextCommand = 1;
      break;
    case 'j':
      mem_base_addr+=4;
      break;
    case 'k':
      mem_base_addr-= 4;
      break;
    case 'b':
      nextBreakpoint = 1;
      break;
    case 'r':
      justRun = 1;
      break;
    case 'p':
      exit(EXIT_SUCCESS);
      break;
    case 'm':
      scanf("%d",&mem_base_addr);
      break;
    case '1':
      showDisplay = showDisplay ^ 1;
        shouldClear++;
      break;
    case '2':
      showCode = showCode ^ 1;
        shouldClear++;
      break;
    case '3':
      showRegister = showRegister ^ 1;
        shouldClear++;
      break;
    case '4':
      showMemory = showMemory ^ 1;
        shouldClear++;
      break;
    case 'q':
        resetCPU(cpu);
    };
  }

  return NULL;
}

void *startDebugger(void *args) {

  CPU *cpu = (CPU *)args;
  printf("Started running Debugger\n");

  init_debugger();
  print_instructions(0);
  refresh();
  wrefresh(win);
  getch();

  pthread_t one, two;

  pthread_create(&one, NULL, threadOne, args);
  pthread_create(&two, NULL, threadTwo, args);

  // -------------------------------------------

  while (1) {
    int i;
    for (i = 0; i < CYCLES_PER_SLEEP; i++) {
      // could upgrade breakpoint lookup to binary search

      for (int j = 0; j < breakpoint_count; j++) {
        if (breakpoints[j] ==
            cpu->pgrm->pc / 4 + 1) { // line numbers are 1-indexed
          nextBreakpoint = 0;
        }
      }

      while ((nextCommand == 0) && (nextBreakpoint == 0) && (justRun == 0)) {
        usleep(100000);
      }
      runCommand(cpu);
      nextCommand = 0;
    }
    usleep(SLEEPTIME);
  }

  // ------------------------------------------

  pthread_join(one, NULL);
  pthread_join(two, NULL);

  printf("Stopped running Debugger\n");
  return NULL;
}
