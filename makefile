s: src/*.c src/*.h src/lib.o
#	cpplint --filter=-whitespace,-legal/copyright,-readability/check,-readability/casting,-build/include,-build/header_guard --recursive src
	gcc -Wall -DDELAY=100 -o s src/*.c src/lib.o -pthread


clean:
	rm -f s


infer:
	infer run -- make

