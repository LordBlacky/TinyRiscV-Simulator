/*
 * TinyRiscV-Simulator 2024
 * ===========================
 *
 * Project: https://github.com/LordBlacky/TinyRiscV-Simulator
 *
 */

#include <curses.h>
#include <ncurses.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cpu.h"
#include "display.h"

#define MAX_LINES 1024           // Maximum number of lines
#define MAX_LINE_LENGTH 1024     // Maximum length of a single line
#define RIGHT_WINDOW_PADDING 140 // Distance to the right side debug info
#define BOTTOM_WINDOW_PADDING 70
#define INSTRUCTION_LINES 10 // Number of instructions shown when debugging
#define NUMBER_OF_INSTRUCTIONS 20

// Array of pointers to hold lines
char *lines[MAX_LINES];
int line_count = 0;

WINDOW *win;

void print_instructions(int line) {
  // Print all lines
  int max_lines = (line + NUMBER_OF_INSTRUCTIONS) < (line_count) ? (line + NUMBER_OF_INSTRUCTIONS) : (line_count);
  for (size_t i = line; i < max_lines; i++) {

    wmove(win, 0 + i - line, RIGHT_WINDOW_PADDING);
    wprintw(win, "%zu: %s", i + 1, lines[i]);
  }

}

void print_display(char (*display)[COLS + 1]) {
  wmove(win, 0, 0);
  int i;
  for(i = 0; i < PAGES * 8; i++){
    wprintw(win, "%s\n", display[i]);
  }

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

void free_lines() {
  // Free allocated memory
  for (size_t i = 0; i < line_count; i++) {
    free(lines[i]);
  }
}

void init_screen() {
  initscr();
  cbreak();
  noecho();

  // make curser invisible
  curs_set(0);

  int res_y = 70;
  int res_x = 200;
  win = newwin(res_y, res_x, 0, 0);
  box(win, 0, 0);
  wmove(win, 1, 1);
  refresh();
  wrefresh(win);
}

void init_debugger() {
  init_screen();
  load_debug_file();

  getch();
  endwin();
}

void *threadOne (void *args) {

  CPU *cpu = (CPU *)args;

  // CODE HERE

  return NULL;

}

void *threadTwo (void *args) {

  CPU *cpu = (CPU *)args;

  // CODE HERE

  return NULL;

}

void *startDebugger (void *args) {

  CPU *cpu = (CPU *)args;
  printf("Started running Debugger\n");

  pthread_t one, two;

  pthread_create(&one,NULL,threadOne,args);
  pthread_create(&two,NULL,threadTwo,args);

 // -------------------------------------------

  init_debugger();
  print_instructions(0);
  refresh();wrefresh(win);
  getch();
  while (1) {
    runCommand(cpu);
    print_display(getPixels());
    print_instructions(cpu->pgrm->pc/4);

    refresh();
    wrefresh(win);
    getch();
  }

  // ------------------------------------------
  
  pthread_join(one,NULL);
  pthread_join(two,NULL);

  printf("Stopped running Debugger\n");
  return NULL;
}

