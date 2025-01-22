simulator:
	gcc display.c debugger.c tinyriscvsimulator.c -o simulator
	./simulator compiled.txt

make compile_asm:
	python3 compiler.py

make clean:
	rm simulator
	rm compiled.txt

