all: calc

calc: main.o calc.o
	gcc main.o calc.o -o calc_make_app

main.o: main.c
	gcc -I. -c main.c

calc.o: src/calc.c
	gcc -I. -c src/calc.c

clean:
	rm -rf *.{o,a} calc_make_app
