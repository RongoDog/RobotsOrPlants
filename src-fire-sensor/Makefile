CC = gcc
CCOPTS = -c -g -Wall -pthread -lpigpio -lrt
LINKOPTS = -g -Wall -pthread -lpigpio -lrt

all: main

main: main.o
	$(CC) $(LINKOPTS) -o $@ $^

main.o: main.c
	$(CC) $(CCOPTS) -o $@ $<

clean:
	rm -rf *.o main main.exe *~
