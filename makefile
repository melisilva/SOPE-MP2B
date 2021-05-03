s: src/*.c src/*.h src/lib.o
	gcc -Wall -DDELAY=100 -o s src/*.c src/lib.o -pthread


clean:
	rm -f s

