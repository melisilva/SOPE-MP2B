s: src/*.c src/*.h src/lib.o
	cpplint --filter=-whitespace,-legal/copyright,-readability/check,-readability/casting,-build/include,-build/header_guard --recursive src
	gcc -Wall -DDELAY=100 -o s src/*.c src/lib.o -pthread


clean:
	rm -f s


infer:
	infer run -- make

#Valgrind calls may be adjusted (in the way it calls our program)

valgrind: s
	 valgrind -q --gen-suppressions=yes --leak-check=full --show-leak-kinds=all --verbose ./s  -t 10 -l 10 /tmp/foo > /dev/null
	 
valgrindf: s
	valgrind -q --gen-suppressions=yes --leak-check=full --show-leak-kinds=all --verbose --log-file=valgrind-out ./s  -t 10 -l 10 /tmp/foo > /dev/null
