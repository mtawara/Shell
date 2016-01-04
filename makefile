CC = g++
DEBUG = -g -O0 -pedantic-errors
CFLAGS = -Wall -std=c++11 -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

all: p3

p3: p3.o
	$(CC) $(LFLAGS) -o p3 p3.o

p3.o: p3.cpp
	$(CC) $(CFLAGS) p3.cpp

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf main
