simulator:
	make clean
	make compile_asm
	make compile
	./simulator compiled.txt 1

compile_asm:
	python3 compiler.py

compile:
	gcc display.c debugger.c tinyriscvsimulator.c -o simulator -lncurses

justcpu:
	make clean
	make compile_asm
	make compile
	./simulator compiled.txt 0

clean:
	touch simulator
	touch compiled.txt
	touch debugger_info.txt
	touch breakpoint_info.txt
	rm simulator
	rm compiled.txt
	rm debugger_info.txt
	rm breakpoint_info.txt

