simulator:
	gcc display.c debugger.c tinyriscvsimulator.c -o simulator -lncurses
	./simulator compiled.txt 1

make compile_asm:
	python3 compiler.py

make clean:
	touch simulator
	touch compiled.txt
	touch debugger_info.txt
	rm simulator
	rm compiled.txt
	rm debugger_info.txt

