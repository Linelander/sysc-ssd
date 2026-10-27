main: main.o
	g++ -o main main.o -L/usr/local/systemc/lib -Wl,-rpath,/usr/local/systemc/lib -lsystemc

main.o: src/main.cpp
	g++ -c -I/usr/local/systemc/include src/main.cpp -o main.o