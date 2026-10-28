main: main.o
	g++ -o main main.o -lsystemc

main.o: src/main.cpp
	g++ -c  src/main.cpp -o main.o

clean:
	rm -rf main *.o