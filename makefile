COMPILER = gcc
FLAGS = -W -Wall

server: main.o
	${COMPILER} ${FLAGS} -pthread -o server main.o

%.o: %.c
	${COMPILER} ${FLAGS} -c -pthread $*.c

clean:
	rm -f *.o *~ server

start:
	./server
