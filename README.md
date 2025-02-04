# TinyRiscV-Simulator
TinyRiscV-Simulator written in C

# Usage
first, clone the repo to your machine.

## 1 Starting
- Put your assembly files into `src/asm/`, make sure there are are only assembly files and optionally Makefiles (these will be ignored)
- in the assembly code you can place breakpoints by writing `#breakpoint` (no whitespace) at the end of a line or in a blank line.
- go to `/src` and run: `make compile_asm`
- this will generate some files for the simulator, including `debug_info.txt`, that is the text that will also be displayed in the simulator, you might want to open in in a text editor as well to look at arbitrary positions in code, since the simulator will only print the current and couple afterwords lines.
- Now run: `make simulator`
  this will compile and run the simulator, make sure you zoom out far enough (Ctrl-) that everything fits on the screen. Your terminal should be able to display about 70 chars vertically.

## 2 Runtime
Use these buttons to controll the simulator:

| Button | Behaviour                       |
| ------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| b:     | run program until next breakpoint                                                                                                                                                                                                                                                                                                                                                                                                                        |
| n:     | run next instruction                                                                                                                                                                                                                                                                                                                                                                                                                                     |
| r:     | run complete complete program, ignoring breakpoints   |  
| q:     | reset CPU: (reset register, reset memory, reset pc)                                                                                                                                                                                                                                                                                                                                                                                                      |
| j/k:   | scroll the memory adresses down/up                                                                                                                                                                                                                                                                                                                                                                                                                       |
| m:     | starts listening to a input number to go to that memory address space<br>to use it, you press m, then enter a number, then press enter.<br>Inputing anything other then number before pressing enter will lead to <br>unexpected behaviour.                                                                                                                                                                                                              |
|        |                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| p:     | quits simulator, though Ctrl-c is preferred and won't break your terminal                                                                                                                                                                                                                                                                                                                                                                                |
|        |                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| 1:     | toggle display                                                                                                                                                                                                                                                                                                                                                                                                                                           |
| 2:     | toggle instructions                                                                                                                                                                                                                                                                                                                                                                                                                                      |
| 3:     | toggle registers                                                                                                                                                                                                                                                                                                                                                                                                                                         |
| 4:     | toggle memory                                                                                                                                                                                                                                                                                                                                                                                                                                            |
|        |                                                                                                                                                                                                                                                                                                                                                                                                                                                          |
| asdf:  | used for gpio. To actually be able to use them, you first run the simulator, then you run `python3 gpio.py`. you need to have the keyboard package installed and on linux it has to be run as root (just sudo probably won't work), because it is essentially a keylogger for these keys. Feel free to look into the code and check for safety. After running you could see in the memory panel at the bottom gpio changing when pressing these buttons. |



## Changing behaviour
If you want to change some things, you can do so in the c files directly. Then recompile.
For example, if you want to increase the speed of the simulator (right now it is fairly slow,
to reduce cpu usage) you can do so by changing the values of `SLEEPTIME` and `CYCLES_PER_SLEEP`
or comment out the `usleep(SLEEPTIME)` completely in start_debugger function in debugger.c.
