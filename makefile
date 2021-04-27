c: src/*.c src/*.h
	gcc -Wall src/*.c -o c -pthread


clean:
	rm -f c



